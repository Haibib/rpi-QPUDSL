#pragma once

namespace qpudsl {

    struct Level;
    struct TensorType;


    struct Expr;
    struct Add;
    struct Bc;
    struct Mul;
    struct Sub;
    struct Sum;
    struct Tensor;
    struct Scalar;


    struct cExpr;
    struct cAdd;
    struct cMul;
    struct cSub;
    struct cTensor;
    struct cScalar;


    struct CIN;
    struct Accumulate;
    struct Assign;
    struct Forall;
    struct Where;

    namespace llir {
        struct lRegType;
        struct Accumulator;
        struct MemoryA;
        struct MemoryB;
        struct Special;

        struct lOpr;
        struct Reg;
        struct Const;
        struct Macro;


        struct lStmt;
        struct Mov;
        struct Shl;
        struct Shr;
        struct Add;
        struct Sub;
        struct Mul;
        struct FAdd;
        struct FSub;
        struct FMul;
        struct Branch;
        struct Sequence;
        struct Label;
        struct SpecialStmt;
        struct RawStmt;
    }

} // namespace qpudsl
