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


// Load 1 16-element tile at ptr_reg into out0.
// Reads 1*16*4 = 64 bytes.
static void emit_dma_load(std::vector<llir::lStmt> &s,
                               llir::lOpr ptr_reg,
                               llir::lOpr vdr_y_off,
                               llir::lOpr vpm_row_base,
                               llir::lOpr out0)
{
    s.push_back(llir::Mov::make(out0, mac("vdr_setup_0(3, 16, 1, vdr_h32(1, 0, 0))")));
    s.push_back(llir::Add::make(mac("vr_setup"), out0, vdr_y_off));
    s.push_back(llir::Mov::make(mac("vr_addr"), ptr_reg));
    s.push_back(llir::Mov::make(mac("-"), mac("vr_wait")));
    s.push_back(llir::Mov::make(out0, mac("vpm_setup(1, 1, h32(0))")));
    s.push_back(llir::Add::make(mac("vr_setup"), out0, vpm_row_base));
    s.push_back(llir::Mov::make(out0, mac("vpm")));
    s.push_back(llir::Mov::make(mac("-"), mac("vr_wait")));
}



static int N_tensors;   // number of input tensors
static int D_dims;      // number of format dimensions

static llir::lOpr get_ptr_reg(int i)        { return ra(i < N_tensors ? i : N_tensors); }
static llir::lOpr get_out_ptr_reg()         { return ra(N_tensors); }
static llir::lOpr slice_start(int i, int d) { return ra(N_tensors + 1 + (i * D_dims) + d); }
static llir::lOpr out_slice_start(int d)    { return ra(N_tensors + 1 + (N_tensors * D_dims) + d); }
static llir::lOpr stride_sizes(int d)       { return ra(N_tensors + 1 + (N_tensors + 1) * D_dims + d); }
static llir::lOpr num_iterations(int d)     { return ra(N_tensors + 1 + (N_tensors + 1) * D_dims + D_dims + d); }
//  rb(1)=qpu<<2 (vpm_setup y: qpu*4), rb(2)=qpu<<6 (vdr_setup_0 y: qpu*4), rb(3)=qpu<<9 (vdw_setup_0 y: qpu*4)
static llir::lOpr loop_counters(int d) { return rb(4+d);}
static llir::lOpr curr_ptr_reg(int i) { return rb(4 + D_dims + i); }


static void emit_preamble(std::vector<llir::lStmt> &s)
{
    // ra layout: [in_ptrs(N)] [out_ptr] [in_slice_starts(N*D)] [out_slice_starts(D)] [stride_sizes(D)] [num_iters(D)]
    internal_assert(N_tensors + 1 + (N_tensors + 1)*D_dims + 2*D_dims - 1 < 32) << "Register alloc is invalid";

    // pointer to tensor start memory
    for (int i = 0; i < N_tensors; ++i)
        s.push_back(llir::Mov::make(get_ptr_reg(i), mac("unif")));
    s.push_back(llir::Mov::make(get_out_ptr_reg(), mac("unif")));

    for (int i = 0; i < N_tensors; ++i) {
        for (int d = 0; d < D_dims; ++d)
            s.push_back(llir::Mov::make(slice_start(i, d), mac("unif")));
    }
    for (int d = 0; d < D_dims; ++d)
        s.push_back(llir::Mov::make(out_slice_start(d), mac("unif")));

    // Dim Sizes (temporary store to rb)
    for(int i=0;i<D_dims;++i)
        s.push_back(llir::Mov::make(rb(i), mac("unif")));
    // Slice Sizes (temporary store to rb)
    for(int i=0;i<D_dims;++i)
        s.push_back(llir::Mov::make(rb(i+D_dims), mac("unif")));

    // qpu_num
    s.push_back(llir::Mov::make(r(1), mac("unif")));

    // QPU distribution: each QPU q handles a contiguous block of the outermost dim.
    // D==1: outermost==innermost; loop iterates in tile units (16 elements each).
    //   Distribute tile_count = sz/16 among 8 QPUs; slice_start offset = t_q * 16 elements.
    // D>=2: outermost is the row dimension; distribute row count = sz among 8 QPUs.
    //   slice_start(i, D-1) offset is in row stride-units (no *16 needed).
    {
        llir::lOpr sz_rb = rb((D_dims - 1) + D_dims);  // = slice_size[D-1]
        if (D_dims == 1) {
            s.push_back(llir::Mov::make(r(0), sz_rb));                             // r0 =sz 
            s.push_back(llir::Shr::make(r(0), r(0), imm(4)));                     // r0 = tile_count= sz/16
            s.push_back(llir::Shr::make(r(2), r(0), imm(3)));                     // r2 = base_tiles
            s.push_back(llir::Shl::make(r(3), r(2), imm(3)));                     // r3 = base_tiles*8
            s.push_back(llir::Sub::make(r(3), r(0), r(3)));                       // r3 = rem_tiles
            s.push_back(llir::Mul::make(r(0), r(1), r(2)));                        // r0 = qpu_num* base_tiles
            s.push_back(llir::Sub::make(r(2), r(1), r(3), llir::FlagsExpr::SetF));// N if qpu_num<rem_tiles
            s.push_back(llir::RawStmt::make("mov.ifn  r2, r1"));                  // if N: r2 = qpu_num
            s.push_back(llir::RawStmt::make("mov.ifnn r2, r3"));                  // if !N: r2 = rem_tiles
            s.push_back(llir::Add::make(r(0), r(0), r(2)));                        // r0 = t_q (start tile)
            s.push_back(llir::Shl::make(r(0), r(0), imm(4)));                     // r0 = t_q*16
            for (int i = 0; i < N_tensors; ++i)
                s.push_back(llir::Add::make(slice_start(i, 0), slice_start(i, 0), r(0)));
            s.push_back(llir::Add::make(out_slice_start(0), out_slice_start(0), r(0)));
        } else {
            s.push_back(llir::Mov::make(r(0), sz_rb));                             
            s.push_back(llir::Shr::make(r(2), r(0), imm(3)));                     // r2 = base_rows = sz>>3
            s.push_back(llir::Shl::make(r(3), r(2), imm(3)));                     // r3 = base_rows*8
            s.push_back(llir::Sub::make(r(3), r(0), r(3)));                       // r3 = rem_rows
            s.push_back(llir::Mul::make(r(0), r(1), r(2)));                         // r0 = qpu_num * base_rows
            s.push_back(llir::Sub::make(r(2), r(1), r(3), llir::FlagsExpr::SetF)); // N if qpu_num < rem_rows
            s.push_back(llir::RawStmt::make("mov.ifn  r2, r1"));                   // if N: r2 = qpu_num
            s.push_back(llir::RawStmt::make("mov.ifnn r2, r3"));                   // if !N: r2 = rem_rows
            s.push_back(llir::Add::make(r(0), r(0), r(2)));                         // r0 = row offset
            for (int i = 0; i < N_tensors; ++i)
                s.push_back(llir::Add::make(slice_start(i, D_dims-1), slice_start(i, D_dims-1), r(0)));
            s.push_back(llir::Add::make(out_slice_start(D_dims-1), out_slice_start(D_dims-1), r(0)));
        }
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
    

    // num_iterations[0] = slice_size[0] / 16 (we processes 1(16 elements) tile at a time)
    // num_iterations[d]  = slice_size[d]  for 0 < d < D-1
    // num_iterations[D-1]  = per-QPU share of slice_size[D-1]
    //                      = sz/8 + (qpu_num < sz%8 ? 1 : 0)
    s.push_back(llir::Mov::make(r(0), rb(0+D_dims)));           
    s.push_back(llir::Shr::make(r(0), r(0), imm(4)));   // r0 = slice_size[0]/16
    for(int d=0;d<D_dims;++d) {
        if(d==D_dims-1) {
            // Per-QPU split: base = r0>>3, rem = r0 - base*8
            // num_iters_q = base + (qpu_num < rem ? 1 : 0)
            s.push_back(llir::Shr::make(r(2), r(0), imm(3)));                      // r2 = base
            s.push_back(llir::Shl::make(r(3), r(2), imm(3)));                      // r3 = base*8
            s.push_back(llir::Sub::make(r(3), r(0), r(3)));                        // r3 = rem
            s.push_back(llir::Sub::make(r(0), r(1), r(3), llir::FlagsExpr::SetF)); // N if qpu_num < rem
            s.push_back(llir::Mov::make(r(0), r(2)));                               // r0 = base
            s.push_back(llir::RawStmt::make("add.ifn r0, r0, 1"));                 // if N: r0 = base+1
        }
        s.push_back(llir::Mov::make(num_iterations(d), r(0)));
        if(d<D_dims-1)
            s.push_back(llir::Mov::make(r(0), rb((d+1)+D_dims)));
    }

    
    // rb(0)=64 tile byte stride, used in body ptr advances
    // rb(1)=qpu<<2 for vpm_setup h32 y 
    // rb(2)=qpu<<6 for vdr_setup_0 vdr_h32 y 
    // rb(3)=qpu<<9 for vdw_setup_0 dma_h32 y
    s.push_back(llir::Mov::make(rb(0), imm(64)));
    s.push_back(llir::Shl::make(rb(1), r(1), imm(2)));
    s.push_back(llir::Shl::make(rb(2), r(1), imm(6)));
    s.push_back(llir::Shl::make(rb(3), r(1), imm(9)));
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

    // Initialize curr_ptr_reg(i) = base_ptr + sum_d(slice_start(i,d) * stride_sizes(d))
    for (int i = 0; i < N_tensors; ++i) {
        s.push_back(llir::Mov::make(curr_ptr_reg(i), get_ptr_reg(i)));
        for (int d = 0; d < D_dims; ++d) {
            s.push_back(llir::Mov::make(r(0), slice_start(i, d)));   
            s.push_back(llir::Mov::make(r(1), stride_sizes(d)));    
            s.push_back(llir::Mul::make(r(0), r(0), r(1)));
            s.push_back(llir::Add::make(curr_ptr_reg(i), curr_ptr_reg(i), r(0)));
        }
    }

    // Initialize output ptr = out_base + sum_d(out_slice_start(d) * stride_sizes(d))
    for (int d = 0; d < D_dims; ++d) {
        s.push_back(llir::Mov::make(r(0), out_slice_start(d)));
        s.push_back(llir::Mov::make(r(1), stride_sizes(d)));
        s.push_back(llir::Mul::make(r(0), r(0), r(1)));
        s.push_back(llir::Add::make(get_out_ptr_reg(), get_out_ptr_reg(), r(0)));
    }


    std::function<void(int)> lower_loop = [&](int d) {

        if (d < 0) {
            // Innermost body: DMA-load all tensors, accumulate, write output.
            for (const EvalStep &step : info.steps) {
                int ti = tensor_idx(step.tensor);
                if (step.is_init) {
                    emit_dma_load(s, curr_ptr_reg(ti), rb(2), rb(1), r(0));
                } else {
                    emit_dma_load(s, curr_ptr_reg(ti), rb(2), rb(1), r(2));
                    if (step.op == OpType::Add)
                        s.push_back(llir::Add::make(r(0), r(0), r(2)));
                    else
                        s.push_back(llir::Mul::make(r(0), r(0), r(2)));
                }
            }

            // Write 1 result tile to output via VPM DMA store
            // rb(1)=qpu<<2: vpm_setup h32 y offset
            // rb(3)=qpu<<9: vdw_setup_0 dma_h32 y offset 
            s.push_back(llir::Mov::make(r(2), mac("vpm_setup(1, 1, h32(0))")));
            s.push_back(llir::Add::make(mac("vw_setup"), r(2), rb(1)));
            s.push_back(llir::Mov::make(mac("vpm"), r(0)));
            s.push_back(llir::Mov::make(r(2), mac("vdw_setup_0(1, 16, dma_h32(0, 0))")));
            s.push_back(llir::Add::make(mac("vw_setup"), r(2), rb(3)));
            s.push_back(llir::Mov::make(mac("vw_addr"), get_out_ptr_reg()));
            s.push_back(llir::Mov::make(mac("-"), mac("vw_wait")));

            // Advance all input ptrs by 64 bytes
            // rb(0)=64
            for (int i = 0; i < N_tensors; ++i) {
                s.push_back(llir::Mov::make(r(0), curr_ptr_reg(i)));
                s.push_back(llir::Add::make(curr_ptr_reg(i), r(0), rb(0)));
            }
            // Advance output ptr
            s.push_back(llir::Add::make(get_out_ptr_reg(), get_out_ptr_reg(), rb(0)));
            return;
        }

        // Loop for dimension d 
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
                // innermost loop always advances 64 bytes
                s.push_back(llir::Mul::make(r(1), r(1), rb(0)));
            } else {
                s.push_back(llir::Mov::make(r(2), stride_sizes(d - 1)));
                s.push_back(llir::Mul::make(r(1), r(1), r(2)));
            }
            s.push_back(llir::Sub::make(r(0), r(0), r(1)));
            for (int i = 0; i < N_tensors; ++i)
                s.push_back(llir::Add::make(curr_ptr_reg(i), curr_ptr_reg(i), r(0)));
            s.push_back(llir::Add::make(get_out_ptr_reg(), get_out_ptr_reg(), r(0)));
        }

        s.push_back(llir::Mov::make(r(0), loop_counters(d)));
        s.push_back(llir::Sub::make(loop_counters(d), r(0), imm(1),
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

} 

llir::lStmt compile_to_llir(const CIN &cin) {
    KernelInfo info = analyze(cin);
    return build_kernel(info);
}

} // namespace qpudsl
