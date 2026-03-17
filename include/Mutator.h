#pragma once

#include "IRFwdDecl.h"

#include <utility>

namespace qpudsl {  

struct Mutator {
    Expr mutate(const Expr &);
    virtual Expr visit(const Add *);
    virtual Expr visit(const Bc *);
    virtual Expr visit(const Mul *);
    virtual Expr visit(const Sub *);
    virtual Expr visit(const Sum *);
    virtual Expr visit(const Tensor *);
    virtual Expr visit(const Scalar *);

    cExpr mutate(const cExpr &);
    virtual cExpr visit(const cAdd *);
    virtual cExpr visit(const cMul *);
    virtual cExpr visit(const cSub *);
    virtual cExpr visit(const cTensor *);
    virtual cExpr visit(const cScalar *);

    CIN mutate(const CIN &);
    virtual CIN visit(const Accumulate *);
    virtual CIN visit(const Assign *);
    virtual CIN visit(const Forall *);
    virtual CIN visit(const Where *);

    private:
    template <typename Base, typename T>
    Base mutate_binop(const T *node) {
        Base a = mutate(node->a);
        Base b = mutate(node->b);
        if (a.same_as(node->a) && b.same_as(node->b)) {
            return node;
        }
        return T::make(std::move(a), std::move(b));
    }
};


#define RESTRICT_MUTATOR(IRType, IRNODE)                                       \
    IRType visit(const IRNODE *) final {                                       \
        internal_error << "Restricted Mutator class does not handle: "         \
                       << typeid(IRNODE).name();                               \
    }

} // namespace qpudsl