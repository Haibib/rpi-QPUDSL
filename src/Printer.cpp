#include "Printer.h"
#include "Scope.h"
#include "Frontend.h"
#include "CIN.h"
#include "llir/LLIR.h"


namespace qpudsl
{   

std::ostream &operator<<(std::ostream &os, const Level &lvl) {
    os << lvl.index;
    return os;
}

std::ostream &operator<<(std::ostream &os, const Format &format) {
    os << "Format{";

    // Ordered levels come first
    if (!format.levels.empty()) {
        os << "[";
        for (size_t i = 0; i < format.levels.size(); ++i) {
            os << format.levels[i];
            if (i + 1 < format.levels.size())
                os << ", ";
        }
        os << "]";
    }

    // Unordered levels
    if (!format.bc_levels.empty()) {
        if (!format.levels.empty())
            os << ", "; // separator only if needed

        os << "{";
        bool first = true;
        for (const auto &lvl : format.bc_levels) {
            if (!first)
                os << ", ";
            os << lvl;
            first = false;
        }
        os << "}";
    }

    // If both empty, show explicitly
    if (format.levels.empty() && format.bc_levels.empty()) {
        os << "empty";
    }

    os << "}";

    return os;
}
    

std::ostream &operator<<(std::ostream &os, const Expr &expr) {
    if (expr.defined()) {
        Printer printer(os);
        printer.print_no_parens(expr);
    } else {
        os << "(undef-expr)";
    }
    return os;
}

std::ostream &operator<<(std::ostream &os, const cExpr &cexpr) {
    if (cexpr.defined()) {
        Printer printer(os);
        printer.print_no_parens(cexpr);
    } else {
        os << "(undef-cexpr)";
    }
    return os;
}

std::ostream &operator<<(std::ostream &os, const CIN &cin) {
    if (cin.defined()) {
        Printer printer(os);
        printer.print(cin);
    } else {
        os << "(undef-cin)";
    }
    return os;
}

void Printer::print(const Expr &expr) {
    ScopedValue<bool> old(implicit_parens, false);
    expr.accept(this);
}

void Printer::print_no_parens(const Expr &expr) {
    ScopedValue<bool> old(implicit_parens, true);
    expr.accept(this);
}

void Printer::visit(const Add *node) {
    open();
    print(node->a);
    os << " + ";
    print(node->b);
    close();
}

void Printer::visit(const Bc *node) {
    os << "bc<" << node->index << ">(";
    print_no_parens(node->a);
    os << ")";
}

void Printer::visit(const Mul *node) {
    open();
    print(node->a);
    os << " * ";
    print(node->b);
    close();
}

void Printer::visit(const Sum *node) {
    os << "sum<" << node->index << ">(";
    print_no_parens(node->a);
    os << ")";
}

void Printer::print_tensor(const std::string &name, const TensorType &type) {
    os << name;
    if (!type.format.levels.empty()) {
        os << "[";
        bool first = true;
        for (const auto &lvl : type.format.levels) {
            if (!first) {
                os << ", ";
            }
            first = false;
            os << lvl.index;
        }
        os << "]";
    }
}

void Printer::visit(const Tensor *node) {
    print_tensor(node->name, node->type);
}


void Printer::print(const cExpr &cexpr) {
    ScopedValue<bool> old(implicit_parens, false);
    cexpr.accept(this);
}

void Printer::print_no_parens(const cExpr &cexpr) {
    ScopedValue<bool> old(implicit_parens, true);
    cexpr.accept(this);
}

void Printer::visit(const cAdd *node) {
    open();
    print(node->a);
    os << " + ";
    print(node->b);
    close();
}

void Printer::visit(const cMul *node) {
    open();
    print(node->a);
    os << " * ";
    print(node->b);
    close();
}

void Printer::visit(const cTensor *node) {
    os << node->name;
    if (!node->slices.empty()) {
        os << "[";
        for (size_t i = 0; i < node->slices.size(); ++i) {
            const auto &sr = node->slices[i];
            if (i > 0) os << ", ";
            if (sr.start == 0 && sr.size == -1) {
                os << ":";
            } else {
                os << sr.start << ":" << (sr.size == -1 ? -1 : sr.start + sr.size);
            }
        }
        os << "]";
    } else {
        // Print with type format levels as before
        if (!node->type.format.levels.empty()) {
            os << "[";
            bool first = true;
            for (const auto &lvl : node->type.format.levels) {
                if (!first) os << ", ";
                first = false;
                os << lvl.index;
            }
            os << "]";
        }
    }
}

void Printer::print(const CIN &cin) { cin.accept(this); }

void Printer::visit(const Accumulate *node) {
    print_indent();
    print_tensor(node->tensor, node->type);
    os << " += ";
    print_no_parens(node->expr);
    end_line();
}

void Printer::visit(const Assign *node) {
    print_indent();
    print_tensor(node->tensor, node->type);
    os << " = ";
    print_no_parens(node->expr);
    end_line();
}

void Printer::visit(const Forall *node) {
    print_indent();
    os << "forall " << node->idx;
    os << "\n";
    indent();
    print(node->body);
    dedent();
}


void Printer::visit(const Where *node) {
    print_indent();
    os << "let " << node->temp << "\n";
    indent();
    print(node->producer);
    dedent();
    print_indent();
    os << "in\n";
    indent();
    print(node->consumer);
    dedent();
}


void Printer::print(const llir::lRegType &expr) {
    expr.accept(this);
}
void Printer::visit(const llir::Accumulator *node) {
    os << "r";
}
void Printer::visit(const llir::MemoryA *node) {
    os << "ra";
}
void Printer::visit(const llir::MemoryB *node) {
    os << "rb";
}
void Printer::visit(const llir::Special *node) {
    switch (node->kind) {
        case llir::Special::UNIF:
            os << "unif";
            break;
        case llir::Special::VR_SETUP:
            os << "vr_setup";
            break;
        case llir::Special::VR_ADDR:
            os << "vr_addr";
            break;
        case llir::Special::VR_WAIT:
            os << "vr_wait";
            break;
        case llir::Special::VPM:
            os << "vpm";
            break;
        case llir::Special::EMPTY:
            os << "-";
            break;
        case llir::Special::INTERRUPT:
            os << "interrupt";
            break;
        default:
            internal_assert(false) << "Unknown Special Register";
            break;
    }

}

void Printer::print(const llir::lOpr &expr) {
    expr.accept(this);
}
void Printer::visit(const llir::Reg * node) {
    print(node->type);
    os<<node->num;
}
void Printer::visit(const llir::Const * node) {
    os<<node->value;
}
void Printer::visit(const llir::Macro * node) {
    os<<node->str;
}

void Printer::print(const llir::FlagsExpr flag) {
    switch (flag)
    {
    case llir::FlagsExpr::None:
        break;
    case llir::FlagsExpr::SetF:
        os<<".setf";
        break;
    case llir::FlagsExpr::AnyNZ:
        os<<".anynz";
        break;
    case llir::FlagsExpr::AllZ:
        os<<".allz";
        break;
    default:
        internal_assert(false) << "Unknown FlagsExpr";
        break;
    }
}

void Printer::print(const llir::lStmt &stmt) {
    stmt.accept(this);
}
void Printer::visit(const llir::Mov * node) {
    print_indent();
    os<<"mov";
    print(node->flags);
    os<<" ";
    print(node->dst);
    os<<", ";
    print(node->src);
    os<<"\n";
}
void Printer::visit(const llir::Shl* node) {
    print_indent();
    os<<"shl";
    print(node->flags);
    os<<" ";
    print(node->dst);
    os<<", ";
    print(node->src);
    os<<", ";
    print(node->shift);
    os<<"\n";
}

void Printer::visit(const llir::Shr* node) {
    print_indent();
    os<<"shr";
    print(node->flags);
    os<<" ";
    print(node->dst);
    os<<", ";
    print(node->src);
    os<<", ";
    print(node->shift);
    os<<"\n";
}

void Printer::visit(const llir::Add* node){
    print_indent();
    os<<"add";
    print(node->flags);
    os<<" ";
    print(node->dst);
    os<<", ";
    print(node->src0);
    os<<", ";
    print(node->src1);
    os<<"\n";
}

void Printer::visit(const llir::Sub* node){
    print_indent();
    os<<"sub";
    print(node->flags);
    os<<" ";
    print(node->dst);
    os<<", ";
    print(node->src0);
    os<<", ";
    print(node->src1);
    os<<"\n";
}

void Printer::visit(const llir::Mul* node){
    print_indent();
    os<<"mul24";
    print(node->flags);
    os<<" ";
    print(node->dst);
    os<<", ";
    print(node->src0);
    os<<", ";
    print(node->src1);
    os<<"\n";
}

void Printer::visit(const llir::Branch * node) {
    print_indent();
    os<<"brr";
    print(node->flags);
    os<<" ";
    print(node->opr);
    os<<", :";
    os<<(node->label);
    os<<"\n";
}

void Printer::visit(const llir::Sequence * node) {
    for(auto stmt: node->stmts) {
        print(stmt);
    }
}

void Printer::visit(const llir::SpecialStmt * node) {
    print_indent();
    switch(node->kind) {
        case llir::SpecialStmt::NOP:
            os<<"nop\n";
            break;
        case llir::SpecialStmt::THREND:
            os<<"thrend\n";
            break;
    }   
}

void Printer::visit(const llir::Label * node) {
    os<<"\n\n";
    print_indent();
    os<<":";
    os<<node->label;
    os<<"\n";
}

void Printer::visit(const llir::RawStmt * node) {
    print_indent();
    os << node->text << "\n";
}


}// namespace qpudsl
