#pragma once

#include "Visitor.h"
#include "Type.h"

#include <iostream>

namespace qpudsl {



std::ostream &operator<<(std::ostream &os, const Level &lvl);
std::ostream &operator<<(std::ostream &os, const Format &format);

std::ostream &operator<<(std::ostream &os, const Expr &expr);

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