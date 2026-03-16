#include "CIN.h"

#include "Error.h"
#include "Printer.h"

#include <algorithm>

namespace qpudsl {

cExpr cAdd::make(cExpr a, cExpr b) {
    internal_assert(a.defined() && b.defined())
        << "Add of undefined: " << a << " + " << b;
    cAdd *node = new cAdd;
    node->a = std::move(a);
    node->b = std::move(b);
    return node;
}

cExpr cMul::make(cExpr a, cExpr b) {
    internal_assert(a.defined() && b.defined())
        << "Mul of undefined: " << a << " + " << b;
    cMul *node = new cMul;
    node->a = std::move(a);
    node->b = std::move(b);
    return node;
}

cExpr cTensor::make(TensorType type, std::string name) {
    internal_assert(!name.empty()) << "Cannot make Tensor with empty name.";
    internal_assert(type.format.bc_levels.empty())
        << "Tensor cannot be made with a type with unordered levels: " << name;
    cTensor *node = new cTensor;
    node->type   = std::move(type);
    node->name   = std::move(name);
    return node;
}

CIN Accumulate::make(std::string tensor, TensorType type, cExpr expr) {
    internal_assert(!tensor.empty()) << "Accumulate with empty tensor";
    internal_assert(expr.defined());

    Accumulate *node = new Accumulate;
    node->tensor = std::move(tensor);
    node->type = std::move(type);
    node->expr = std::move(expr);
    return node;
}

CIN Assign::make(std::string tensor, TensorType type, cExpr expr) {
    internal_assert(!tensor.empty()) << "Assign with empty tensor";
    internal_assert(expr.defined());

    Assign *node = new Assign;
    node->tensor = std::move(tensor);
    node->type = std::move(type);
    node->expr = std::move(expr);
    return node;
}

CIN Forall::make(std::string idx, CIN body) {
    internal_assert(!idx.empty()) << "Forall with empty index";
    internal_assert(body.defined())
        << "Forall with undef body for: " << idx ;

    Forall *node = new Forall;
    node->idx = std::move(idx);
    node->body = std::move(body);
    return node;
}

CIN Where::make(std::string temp, TensorType temp_type, CIN producer,
                CIN consumer) {
    internal_assert(!temp.empty()) << "Where with empty temp";
    // TODO: assert temp_type is scalar?
    internal_assert(producer.defined())
        << "Where with undef producer for temp: " << temp;
    // TODO: assert producer writes to temp
    internal_assert(consumer.defined())
        << "Where with undef consumer for temp: " << temp;
    // TODO: assert consumer reads from temp

    Where *node = new Where;
    node->temp = std::move(temp);
    node->temp_type = std::move(temp_type);
    node->producer = std::move(producer);
    node->consumer = std::move(consumer);
    return node;
}

} // namespace qpudsl
