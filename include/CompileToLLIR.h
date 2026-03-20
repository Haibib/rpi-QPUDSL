#pragma once

#include "CIN.h"
#include "Type.h"
#include "llir/LLIR.h"

#include <string>
#include <vector>

namespace qpudsl {

enum class OpType { Add, Sub, Mul };

struct EvalStep {
    std::string tensor;
    std::string fused_scalar;
    bool        is_init;
    bool        is_scalar;
    OpType      op;
};

struct TensorInfo {
    std::string name;
};

struct KernelInfo {
    std::vector<EvalStep>    steps;
    std::vector<TensorInfo>  tensors;    // eval-order input tensors
    std::vector<std::string> scalars;
    std::string              out_tensor;
    int                      D;          // number of dimensions
    dType                    dtype;
};

KernelInfo  analyze_kernel(const CIN &cin);
llir::lStmt compile_to_llir(const CIN &cin);

} // namespace qpudsl
