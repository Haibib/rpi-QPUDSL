#include "CompileToLLIR.h"

#include "CIN.h"
#include "Error.h"
#include "Visitor.h"
#include "llir/LLIR.h"

#include <algorithm>
#include <vector>
#include <string>
#include <cstdint>

namespace qpudsl {

namespace {


static llir::lOpr r(int n)        { return llir::Reg::make(llir::Accumulator::make(), n); }
static llir::lOpr ra(int n)       { return llir::Reg::make(llir::MemoryA::make(), n); }
static llir::lOpr rb(int n)       { return llir::Reg::make(llir::MemoryB::make(), n); }
static llir::lOpr imm(int64_t v)  { return llir::Const::make(v); }
static llir::lOpr mac(std::string s) { return llir::Macro::make(std::move(s)); }
static llir::lStmt nop()          { return llir::SpecialStmt::make(llir::SpecialStmt::NOP); }


enum class OpType { Add, Mul };

struct EvalStep {
    std::string tensor;
    bool        is_init;
    OpType      op;
};

void flatten_into(const cExpr &e, std::vector<EvalStep> &steps) {
    if (const cTensor *t = e.as<cTensor>()) {
        steps.push_back({t->name, true, OpType::Add});
        return;
    }
    OpType op;
    const cExpr *lhs = nullptr, *rhs = nullptr;
    if (const cAdd *a = e.as<cAdd>())      { op = OpType::Add; lhs = &a->a; rhs = &a->b; }
    else if (const cMul *m = e.as<cMul>()) { op = OpType::Mul; lhs = &m->a; rhs = &m->b; }
    else internal_assert(false) << "Unknown cExpr node";

    const bool lhs_leaf = (lhs->as<cTensor>() != nullptr);
    const bool rhs_leaf = (rhs->as<cTensor>() != nullptr);

    if (lhs_leaf && rhs_leaf) {
        steps.push_back({lhs->as<cTensor>()->name, true,  OpType::Add});
        steps.push_back({rhs->as<cTensor>()->name, false, op});
    } else if (!lhs_leaf && rhs_leaf) {
        flatten_into(*lhs, steps);
        steps.push_back({rhs->as<cTensor>()->name, false, op});
    } else if (lhs_leaf && !rhs_leaf) {
        flatten_into(*rhs, steps);
        steps.push_back({lhs->as<cTensor>()->name, false, op});
    } else {
        internal_assert(false)
            << "non-linear expression trees require multiple accumulators (not yet supported).";
    }
}


struct TensorInfo {
    std::string             name;
    std::vector<SliceRange> slices;
};

struct KernelInfo {
    std::vector<EvalStep>   steps;
    std::vector<TensorInfo> tensors;   // input tensors, unique, in eval order
    std::string             out_tensor;
    int                     D;         // number of dimensions
};

KernelInfo analyze(const CIN &cin) {
    CIN body = cin;
    while (const Forall *f = body.as<Forall>()) { body = f->body; }

    const Assign *assign = body.as<Assign>();
    internal_assert(assign)
        << "expected Assign Stmt.";

    std::vector<EvalStep> steps;
    flatten_into(assign->expr, steps);
    internal_assert(!steps.empty()) << "empty expression.";

    // Collect unique tensors in eval order.
    struct InfoCollector : public Visitor {
        std::vector<TensorInfo> tensors;
        int D = 0;
        bool found_D = false;
        void visit(const cTensor *t) override {
            if (!found_D) {
                D = (int)t->type.format.levels.size();
                found_D = true;
            }
            for (const auto &ti : tensors)
                if (ti.name == t->name) return;
            tensors.push_back({t->name, t->slices});
        }
    };
    InfoCollector col;
    assign->expr.accept(&col);

    internal_assert(col.found_D) << "no Tensor in expression.";

    // Re-order tensors to match eval step order.
    std::vector<TensorInfo> ordered;
    for (const auto &st : steps) {
        for (const auto &ti : col.tensors) {
            if (ti.name != st.tensor) continue;
            bool dup = false;
            for (const auto &ot : ordered) if (ot.name == ti.name) { dup = true; break; }
            if (!dup) ordered.push_back(ti);
            break;
        }
    }



    int D  = col.D;
    int NL = std::max(D - 1, 1);

    return KernelInfo{std::move(steps), std::move(ordered), assign->tensor, D};
}


// Load 2 16-width vectors at ptr_reg into out0/out1. 
// Reads 2*16*4 = 128 bytes.
static void emit_dma_load(std::vector<llir::lStmt> &s,
                               llir::lOpr ptr_reg,
                               llir::lOpr vdr_y_off,
                               llir::lOpr vpm_row_base,
                               llir::lOpr out0,
                               llir::lOpr out1)
{
    s.push_back(llir::Mov::make(out0, mac("vdr_setup_0(3, 16, 2, vdr_h32(1, 0, 0))")));
    s.push_back(llir::Add::make(mac("vr_setup"), out0, vdr_y_off));
    s.push_back(llir::Mov::make(mac("vr_addr"), ptr_reg));
    s.push_back(llir::Mov::make(mac("-"), mac("vr_wait")));
    s.push_back(llir::Mov::make(out0, mac("vpm_setup(2, 1, h32(0))")));
    s.push_back(llir::Add::make(mac("vr_setup"), out0, vpm_row_base));
    s.push_back(llir::Mov::make(out0, mac("vpm")));
    s.push_back(llir::Mov::make(out1, mac("vpm")));
    s.push_back(llir::Mov::make(mac("-"), mac("vr_wait")));
}



static int N_tensors;   // number of input tensors
static int D_dims;      // number of format dimensions

static llir::lOpr get_ptr_reg(int i)       { return ra(i < N_tensors ? i : N_tensors); }
static llir::lOpr get_out_ptr_reg()        { return ra(N_tensors); }
static llir::lOpr slice_start(int i, int d) { return ra(N_tensors + 1 + (i * D_dims) + d); }
static llir::lOpr stride_sizes(int d) { return ra(N_tensors + 1 + (N_tensors * D_dims) + d); }
static llir::lOpr num_iterations(int d) { return ra(N_tensors + 1 + (N_tensors * D_dims) + D_dims + d); }
//  rb(1)=qpu<<2, rb(2)=qpu<<6, rb(3)=qpu<<7
static llir::lOpr loop_counters(int d) { return rb(4+d);}
static llir::lOpr curr_ptr_reg(int i) { return rb(4 + D_dims + i); }


static void emit_preamble(std::vector<llir::lStmt> &s)
{
    internal_assert(N_tensors + 1 + (N_tensors*D_dims) + 2*D_dims - 1 < 32) << "Register alloc is invalid";

    // pointer to tensor start memory
    for (int i = 0; i < N_tensors; ++i)
        s.push_back(llir::Mov::make(get_ptr_reg(i), mac("unif")));
    s.push_back(llir::Mov::make(get_out_ptr_reg(), mac("unif")));

    for (int i = 0; i < N_tensors; ++i) {
        for (int d = 0; d < D_dims; ++d)
            s.push_back(llir::Mov::make(slice_start(i, d), mac("unif")));
    }

    // Dim Sizes (temporary store to rb)
    for(int i=0;i<D_dims;++i)
        s.push_back(llir::Mov::make(rb(i), mac("unif")));
    // Slice Sizes (temporary store to rb)
    for(int i=0;i<D_dims;++i)
        s.push_back(llir::Mov::make(rb(i+D_dims), mac("unif")));

    // qpu_num
    s.push_back(llir::Mov::make(r(1), mac("unif")));

    for(int i=0;i<N_tensors;++i) {
        // divide outermost dim (D_dims-1) in 8 parts for 8 qpus
        s.push_back(llir::Shr::make(r(2), rb((D_dims-1)+D_dims), imm(3)));
        s.push_back(llir::Mul::make(r(2), r(2), r(1)));
        s.push_back(llir::Add::make(slice_start(i, D_dims-1), slice_start(i, D_dims-1), r(2)));
    }

    // find stride sizes for all dimensions
    // stride_size[0]=4 cause each element is 4 bytes
    s.push_back(llir::Mov::make(stride_sizes(0), imm(4)));

    if(D_dims>1) {
        // stride_size[1] = dim_size[0]*4 (number of bytes in the innermost row)
        // stride_size[i] = stride_size[i-1]*dim_size[i-1] for i>1
        s.push_back(llir::Mov::make(r(0), imm(4)));
        for(int d=1;d<D_dims;++d) {
            s.push_back(llir::Mul::make(r(0), r(0), rb(d-1)));
            s.push_back(llir::Mov::make(stride_sizes(d), r(0)));
        }
    }
    

    // num_iterations[0] = slice_size[0]/32
    // num_iterations[D_dims-1] = slice_size[D_dims-1]/8
    // num_iterations[i] = slice_size[i] for other dims
    // if D==1 then num_iterations[0] = slice_size[0]/32/8
    s.push_back(llir::Shr::make(r(0), rb(0+D_dims),imm(5))); 
    for(int d=0;d<D_dims;++d) {
        if(d==D_dims-1) {
            s.push_back(llir::Shr::make(r(0), r(0), imm(3)));
        }
        s.push_back(llir::Mov::make(num_iterations(d), r(0)));
        s.push_back(llir::Mov::make(r(0), rb((d+1)+D_dims)));
    }

    
    // rb(1)=qpu<<2, rb(2)=qpu<<6, rb(3)=qpu<<7 — used as VPM row offsets in emit_dma_load
    s.push_back(llir::Shl::make(rb(1), r(1), imm(2)));
    s.push_back(llir::Shl::make(rb(2), r(1), imm(6)));
    s.push_back(llir::Shl::make(rb(3), r(1), imm(7)));
}


llir::lStmt build_kernel(const KernelInfo &info) {
    N_tensors  = (int)info.tensors.size();
    D_dims  = info.D;


    std::vector<llir::lStmt> s;   

    s.push_back(llir::RawStmt::make(".include \"../share/vc4inc/vc4.qinc\""));

    emit_preamble(s);

    // Tensor-index lookup helper
    auto tensor_idx = [&](const std::string &name) -> int {
        for (int i = 0; i < N_tensors; ++i)
            if (info.tensors[i].name == name) return i;
        internal_assert(false) << "tensor not found: " << name;
        return -1;
    };

    // Initialize curr_ptr_reg(i) = base_ptr + slice_offset
    // byte_offset = sum_d slice_start(i,d) * stride_sizes(d)
    for (int i = 0; i < N_tensors; ++i) {
        s.push_back(llir::Mov::make(curr_ptr_reg(i), get_ptr_reg(i)));
        for (int d = 0; d < D_dims; ++d) {
            s.push_back(llir::Mov::make(r(0), slice_start(i, d)));   // ra -> acc
            s.push_back(llir::Mov::make(r(1), stride_sizes(d)));     // ra -> acc
            s.push_back(llir::Mul::make(r(0), r(0), r(1)));
            s.push_back(llir::Add::make(curr_ptr_reg(i), curr_ptr_reg(i), r(0)));
        }
    }

    // D nested loops: lower_loop(D-1)=outermost, lower_loop(0)=innermost, lower_loop(-1)=body
    // lower_loop(d) maps 1:1 to preamble dimension d.
    std::function<void(int)> lower_loop = [&](int d) {

        if (d < 0) {
            // Innermost body: DMA-load all tensors, accumulate, write output.
            for (const EvalStep &step : info.steps) {
                int ti = tensor_idx(step.tensor);
                if (step.is_init) {
                    // First tensor: load 2 tiles (128 bytes) into r(0), r(1)
                    emit_dma_load(s, curr_ptr_reg(ti), rb(2), rb(3), r(0), r(1));
                } else {
                    // Subsequent tensor: load into r(2), r(3), apply op to r(0)/r(1)
                    emit_dma_load(s, curr_ptr_reg(ti), rb(2), rb(3), r(2), r(3));
                    if (step.op == OpType::Add) {
                        s.push_back(llir::Add::make(r(0), r(0), r(2)));
                        s.push_back(llir::Add::make(r(1), r(1), r(3)));
                    } else {
                        s.push_back(llir::Mul::make(r(0), r(0), r(2)));
                        s.push_back(llir::Mul::make(r(1), r(1), r(3)));
                    }
                }
            }

            // Write 2 result tiles to output via VPM DMA store
            s.push_back(llir::Mov::make(r(2), mac("vpm_setup(2, 1, h32(0))")));
            s.push_back(llir::Add::make(mac("vw_setup"), r(2), rb(3)));
            s.push_back(llir::Mov::make(mac("vpm"), r(0)));
            s.push_back(llir::Mov::make(mac("vpm"), r(1)));
            s.push_back(llir::Mov::make(r(2), mac("vdw_setup_0(2, 16, dma_h32(1, 0))")));
            s.push_back(llir::Add::make(mac("vw_setup"), r(2), rb(3)));
            s.push_back(llir::Mov::make(mac("vw_addr"), get_out_ptr_reg()));
            s.push_back(llir::Mov::make(mac("-"), mac("vw_wait")));

            // Advance all input ptrs by 128 bytes (32 elements * 4 bytes)
            for (int i = 0; i < N_tensors; ++i)
                s.push_back(llir::Add::make(curr_ptr_reg(i), curr_ptr_reg(i), imm(128)));
            // Advance output ptr by 128 bytes (contiguous, no correction needed)
            s.push_back(llir::Add::make(get_out_ptr_reg(), get_out_ptr_reg(), imm(128)));
            return;
        }

        // Loop for dimension d (count-down from num_iterations(d) to 0)
        s.push_back(llir::Mov::make(loop_counters(d), num_iterations(d)));
        std::string lbl = "loop_" + std::to_string(d);
        s.push_back(llir::Label::make(lbl));

        lower_loop(d - 1);

        // Input ptr correction: after inner iterations, curr_ptr advanced by
        // num_iterations(d-1)*stride_sizes(d-1). Need total = stride_sizes(d).
        // correction = stride_sizes(d) - num_iterations(d-1)*stride_sizes(d-1).
        // Output ptr needs no correction (contiguous layout).
        if (d > 0) {
            s.push_back(llir::Mov::make(r(0), stride_sizes(d)));
            s.push_back(llir::Mov::make(r(1), num_iterations(d - 1)));
            if (d == 1) {
                // body always advances 128 bytes; stride_sizes(0)=4 is per-element, not per-step
                s.push_back(llir::Mul::make(r(1), r(1), imm(128)));
            } else {
                s.push_back(llir::Mov::make(r(2), stride_sizes(d - 1)));
                s.push_back(llir::Mul::make(r(1), r(1), r(2)));
            }
            s.push_back(llir::Sub::make(r(0), r(0), r(1)));
            for (int i = 0; i < N_tensors; ++i)
                s.push_back(llir::Add::make(curr_ptr_reg(i), curr_ptr_reg(i), r(0)));
        }

        s.push_back(llir::Sub::make(loop_counters(d), loop_counters(d), imm(1),
                                    llir::FlagsExpr::SetF));
        s.push_back(llir::Branch::make(mac("-"), lbl, llir::FlagsExpr::AnyNZ));
        s.push_back(nop()); s.push_back(nop()); s.push_back(nop());
    };

    lower_loop(D_dims - 1);

    // Thread-end sequence
    s.push_back(llir::SpecialStmt::make(llir::SpecialStmt::THREND));
    s.push_back(nop()); s.push_back(nop());

    return llir::Sequence::make(std::move(s));
}

} // anonymous namespace

llir::lStmt compile_to_llir(const CIN &cin) {
    KernelInfo info = analyze(cin);
    return build_kernel(info);
}

} // namespace qpudsl
