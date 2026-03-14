#include "Visitor.h"
#include "Frontend.h"
#include "CIN.h"
#include "llir/LLIR.h"

namespace qpudsl {

void Visitor::visit(const Add *node) {
    node->a.accept(this);
    node->b.accept(this);
}

void Visitor::visit(const Bc *node) { node->a.accept(this); }

void Visitor::visit(const Mul *node) {
    node->a.accept(this);
    node->b.accept(this);
}

void Visitor::visit(const Sum *node) { node->a.accept(this); }

void Visitor::visit(const Tensor *node) { (void)node; }

void Visitor::visit(const cAdd *node) {
    node->a.accept(this);
    node->b.accept(this);
}

void Visitor::visit(const cMul *node) {
    node->a.accept(this);
    node->b.accept(this);
}

void Visitor::visit(const cTensor *node) { (void)node; }

void Visitor::visit(const Accumulate *node) { node->expr.accept(this); }

void Visitor::visit(const Assign *node) { node->expr.accept(this); }

void Visitor::visit(const Forall *node) {
    node->body.accept(this);
}

void Visitor::visit(const Where *node) {
    node->producer.accept(this);
    node->consumer.accept(this);
}


void Visitor::visit(const llir::Accumulator *node) { (void)node; }
void Visitor::visit(const llir::MemoryA *node) { (void)node; }
void Visitor::visit(const llir::MemoryB *node) { (void)node; }
void Visitor::visit(const llir::Special *node) { (void)node; }

void Visitor::visit(const llir::Reg *node) {
    node->type.accept(this);
}
void Visitor::visit(const llir::Const *node) { (void)node; }
void Visitor::visit(const llir::Macro *node) { (void)node; }

void Visitor::visit(const llir::Mov *node) {
    node->dst.accept(this);
    node->src.accept(this);
}

void Visitor::visit(const llir::Shl *node) {
    node->dst.accept(this);
    node->src.accept(this);
    node->shift.accept(this);
}

void Visitor::visit(const llir::Add *node) {
    node->dst.accept(this);
    node->src0.accept(this);
    node->src1.accept(this);
}

void Visitor::visit(const llir::Sub *node) {
    node->dst.accept(this);
    node->src0.accept(this);
    node->src1.accept(this);
}

void Visitor::visit(const llir::Mul *node) {
    node->dst.accept(this);
    node->src0.accept(this);
    node->src1.accept(this);
}

void Visitor::visit(const llir::Branch *node) {
    (void)node;
}

void Visitor::visit(const llir::Sequence *node) {
    for (const auto &stmt : node->stmts) {
        stmt.accept(this);
    }
}

void Visitor::visit(const llir::SpecialStmt *node) {
    (void)node;
}

void Visitor::visit(const llir::Label * node) {
    (void)node;
}

void Visitor::visit(const llir::RawStmt * node) {
    (void)node;
}

} // namespace qpudsl