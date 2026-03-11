#pragma once
#include "IRFwdDecl.h"
#include "Visitor.h"
#include "Type.h"
#include "CIN.h"
#include "Frontend.h"

#include <iostream>

namespace qpudsl {



std::ostream &operator<<(std::ostream &os, const Level &lvl);
std::ostream &operator<<(std::ostream &os, const Format &format);

std::ostream &operator<<(std::ostream &os, const Expr &expr);

std::ostream &operator<<(std::ostream &os, const cExpr &cexpr);
std::ostream &operator<<(std::ostream &os, const CIN &cin);

struct Printer : public Visitor {
    explicit Printer(std::ostream &os) : os(os) {}

    virtual void print(const Expr &);
    virtual void print_no_parens(const Expr &);
    virtual void visit(const Add *) override;
    virtual void visit(const Bc *) override;
    virtual void visit(const Mul *) override;
    virtual void visit(const Sum *) override;
    virtual void print_tensor(const std::string &name, const TensorType &type);
    virtual void visit(const Tensor *) override;

    virtual void print(const cExpr &);
    virtual void print_no_parens(const cExpr &);
    virtual void visit(const cAdd *) override;
    virtual void visit(const cMul *) override;
    virtual void visit(const cTensor *) override;

    virtual void print(const CIN &);
    virtual void visit(const Accumulate *) override;
    virtual void visit(const Assign *) override;
    virtual void visit(const Forall *) override;
    virtual void visit(const Where *) override;


    virtual void print(const llir::lRegType &);
    virtual void visit(const llir::Accumulator *) override;
    virtual void visit(const llir::MemoryA *) override;
    virtual void visit(const llir::MemoryB *) override;
    virtual void visit(const llir::Special *) override;

    virtual void print(const llir::lOpr &);
    virtual void visit(const llir::Reg *) override;
    virtual void visit(const llir::Const *) override;
    virtual void visit(const llir::Macro *) override;

    virtual void print(const llir::lStmt &);
    virtual void visit(const llir::Mov *) override;
    virtual void visit(const llir::Shl *) override;
    virtual void visit(const llir::Add *) override;
    virtual void visit(const llir::Sub *) override;
    virtual void visit(const llir::Mul *) override;
    virtual void visit(const llir::Branch *) override;
    virtual void visit(const llir::Sequence *) override;
    virtual void visit(const llir::SpecialStmt *) override;
    virtual void visit (const llir::Label *) override;
    virtual void print(const llir::FlagsExpr flag);
    void indent() { indent_count++; }
    void dedent() { indent_count--; }

  private:
    /** The stream on which we're outputting */
    std::ostream &os;

    // false -> print parens on open()/close()
    bool implicit_parens = true;

    void open() const {
        if (!implicit_parens) {
            os << "(";
        }
    }

    void close() const {
        if (!implicit_parens) {
            os << ")";
        }
    }

    size_t indent_count = 0;

    virtual void print_indent() {
        for (size_t i = 0; i < indent_count; i++) {
            os << "  "; // two spaces
        }
    }

    virtual void end_line() { os << "\n"; }
};

} // namespace qpudsl