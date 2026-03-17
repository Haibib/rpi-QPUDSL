#include "Mutator.h"

#include "CIN.h"
#include "Frontend.h"

namespace qpudsl {

Expr Mutator::mutate(const Expr &expr) {
    return expr.defined() ? expr.get()->mutate_Expr(this) : Expr();
}

Expr Mutator::visit(const Add *node) { return mutate_binop<Expr>(node); }

Expr Mutator::visit(const Bc *node) {
    Expr a = mutate(node->a);
    if (a.same_as(node->a)) {
        return node;
    }
    return Bc::make(node->index, std::move(a));
}

Expr Mutator::visit(const Mul *node) { return mutate_binop<Expr>(node); }

Expr Mutator::visit(const Sum *node) {
    Expr a = mutate(node->a);
    if (a.same_as(node->a)) {
        return node;
    }
    return Sum::make(node->index, std::move(a));
}

Expr Mutator::visit(const Tensor *node) { return node; }

Expr Mutator::visit(const Sub *node) { return mutate_binop<Expr>(node); }

Expr Mutator::visit(const Scalar *node) { return node; }


cExpr Mutator::mutate(const cExpr &cexpr) {
    return cexpr.defined() ? cexpr.get()->mutate_cExpr(this) : cExpr();
}

cExpr Mutator::visit(const cAdd *node) { return mutate_binop<cExpr>(node); }

cExpr Mutator::visit(const cMul *node) { return mutate_binop<cExpr>(node); }

cExpr Mutator::visit(const cSub *node) { return mutate_binop<cExpr>(node); }

cExpr Mutator::visit(const cTensor *node) { return node; }

cExpr Mutator::visit(const cScalar *node) { return node; }

CIN Mutator::mutate(const CIN &cin) {
    return cin.defined() ? cin.get()->mutate_CIN(this) : CIN();
}

CIN Mutator::visit(const Accumulate *node) {
    cExpr expr = mutate(node->expr);
    if (expr.same_as(node->expr)) {
        return node;
    }
    return Accumulate::make(node->tensor, node->type, std::move(expr));
}

CIN Mutator::visit(const Assign *node) {
    cExpr expr = mutate(node->expr);
    if (expr.same_as(node->expr)) {
        return node;
    }
    return Assign::make(node->tensor, node->type, std::move(expr));
}

CIN Mutator::visit(const Forall *node) {
    CIN body = mutate(node->body);
    return Forall::make(node->idx,  std::move(body));
}


CIN Mutator::visit(const Where *node) {
    CIN producer = mutate(node->producer);
    CIN consumer = mutate(node->consumer);
    if (producer.same_as(node->producer) && consumer.same_as(node->consumer)) {
        return node;
    }
    return Where::make(node->temp, node->temp_type, std::move(producer),
                       std::move(consumer));
}

} // namespace qpudsl
