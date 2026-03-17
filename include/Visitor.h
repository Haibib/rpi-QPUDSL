#pragma once

#include "IRFwdDecl.h"

namespace qpudsl {

struct Visitor {
    // Expr
    virtual void visit(const Add *);
    virtual void visit(const Bc *);
    virtual void visit(const Mul *);
    virtual void visit(const Sub *);
    virtual void visit(const Sum *);
    virtual void visit(const Tensor *);
    virtual void visit(const Scalar *);

     // cExpr
    virtual void visit(const cAdd *);
    virtual void visit(const cMul *);
    virtual void visit(const cSub *);
    virtual void visit(const cTensor *);
    virtual void visit(const cScalar *);

    // CIN
    virtual void visit(const Accumulate *);
    virtual void visit(const Assign *);
    virtual void visit(const Forall *);
    virtual void visit(const Where *);

    //LLIR
    virtual void visit(const llir::Accumulator *);
    virtual void visit(const llir::MemoryA *);
    virtual void visit(const llir::MemoryB *);
    virtual void visit(const llir::Special *);

    virtual void visit(const llir::Reg *);
    virtual void visit(const llir::Const *);
    virtual void visit(const llir::Macro *);

    virtual void visit(const llir::Mov *);
    virtual void visit(const llir::Shl *);
    virtual void visit(const llir::Shr *);
    virtual void visit(const llir::Add *);
    virtual void visit(const llir::Sub *);
    virtual void visit(const llir::Mul *);
    virtual void visit(const llir::FAdd *);
    virtual void visit(const llir::FSub *);
    virtual void visit(const llir::FMul *);
    virtual void visit(const llir::Branch *);
    virtual void visit(const llir::Sequence *);
    virtual void visit(const llir::SpecialStmt *);
    virtual void visit(const llir::Label *);
    virtual void visit(const llir::RawStmt *);
};

#define RESTRICT_VISITOR(IRNODE)                                               \
    void visit(const IRNODE *) final {                                         \
        internal_error << "Restricted Visitor class does not handle: "         \
                       << typeid(IRNODE).name();                               \
    }

} // namespace qpudsl
