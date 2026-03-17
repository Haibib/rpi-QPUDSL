#include "llir/LLIR.h"

#include "Error.h"
#include "Printer.h"

#include <algorithm>

namespace qpudsl {
namespace llir {


lRegType Accumulator::make() {
    Accumulator *node = new Accumulator;
    return node;
}

lRegType MemoryA::make() {
    MemoryA *node = new MemoryA;
    return node;
}

lRegType MemoryB::make() {
    MemoryB *node = new MemoryB;
    return node;
}

lRegType Special::make(Special::Kind kind) {
    Special *node = new Special;
    node->kind = std::move(kind);
    return node;
}

lOpr Reg::make(lRegType type, int num) {
    internal_assert(type.defined())
        << "Cannot make Reg with undefined type";
    internal_assert(num >= 0) << "Cannot make Reg with negative num";
    Reg *node = new Reg;
    node->type = std::move(type);
    node->num = num;
    return node;
}


lOpr Const::make(int64_t value) {
    Const *node = new Const;
    node->value = value;
    return node;
}

lOpr Macro::make(std::string str) {
    Macro *node = new Macro;
    node->str = std::move(str);
    return node;
}


lStmt Mov::make(lOpr dst, lOpr src, FlagsExpr flags) {
    internal_assert(dst.defined())
        << "Cannot make Mov with undefined dst";
    internal_assert(src.defined())
        << "Cannot make Mov with undefined src";
    Mov *node = new Mov;
    node->dst = std::move(dst);
    
    node->src = std::move(src);
    node->flags = flags;
    return node;
}

lStmt Shl::make(lOpr dst, lOpr src0, lOpr src1, FlagsExpr flags) {
    internal_assert(dst.defined())
        << "Cannot make Shl with undefined dst";
    internal_assert(src0.defined())
        << "Cannot make Shl with undefined src0";
    internal_assert(src1.defined())
        << "Cannot make Shl with undefined src1";
    Shl *node = new Shl;
    node->dst   = std::move(dst);
    node->src   = std::move(src0);
    node->shift = std::move(src1);
    node->flags = flags;
    return node;
} 

lStmt Shr::make(lOpr dst, lOpr src0, lOpr src1, FlagsExpr flags) {
    internal_assert(dst.defined())
        << "Cannot make Shr with undefined dst";
    internal_assert(src0.defined())
        << "Cannot make Shr with undefined src0";
    internal_assert(src1.defined())
        << "Cannot make Shr with undefined src1";
    Shr *node = new Shr;
    node->dst   = std::move(dst);
    node->src   = std::move(src0);
    node->shift = std::move(src1);
    node->flags = flags;
    return node;
}

lStmt Add::make(lOpr dst, lOpr src0, lOpr src1, FlagsExpr flags) {
    internal_assert(dst.defined())
        << "Cannot make Add with undefined dst";
    internal_assert(src0.defined())
        << "Cannot make Add with undefined src0";
    internal_assert(src1.defined())
        << "Cannot make Add with undefined src1";
    Add *node = new Add;
    node->dst = std::move(dst);
    node->src0 = std::move(src0);
    node->src1 = std::move(src1);
    node->flags = flags;
    return node;
}

lStmt Sub::make(lOpr dst, lOpr src0, lOpr src1, FlagsExpr flags) {
    internal_assert(dst.defined())
        << "Cannot make Sub with undefined dst";
    internal_assert(src0.defined())
        << "Cannot make Sub with undefined src0";
    internal_assert(src1.defined())
        << "Cannot make Sub with undefined src1";
    Sub *node = new Sub;
    node->dst = std::move(dst);
    node->src0 = std::move(src0);
    node->src1 = std::move(src1);
    node->flags = flags;
    return node;
}

lStmt Mul::make(lOpr dst, lOpr src0, lOpr src1, FlagsExpr flags) {
    internal_assert(dst.defined())
        << "Cannot make Mul with undefined dst";
    internal_assert(src0.defined())
        << "Cannot make Mul with undefined src0";
    internal_assert(src1.defined())
        << "Cannot make Mul with undefined src1";
    Mul *node = new Mul;
    node->dst = std::move(dst);
    node->src0 = std::move(src0);
    node->src1 = std::move(src1);
    node->flags = flags;
    return node;
}


lStmt FAdd::make(lOpr dst, lOpr src0, lOpr src1, FlagsExpr flags) {
    FAdd *node = new FAdd;
    node->dst = std::move(dst);
    node->src0 = std::move(src0);
    node->src1 = std::move(src1);
    node->flags = flags;
    return node;
}

lStmt FSub::make(lOpr dst, lOpr src0, lOpr src1, FlagsExpr flags) {
    FSub *node = new FSub;
    node->dst = std::move(dst);
    node->src0 = std::move(src0);
    node->src1 = std::move(src1);
    node->flags = flags;
    return node;
}

lStmt FMul::make(lOpr dst, lOpr src0, lOpr src1, FlagsExpr flags) {
    FMul *node = new FMul;
    node->dst = std::move(dst);
    node->src0 = std::move(src0);
    node->src1 = std::move(src1);
    node->flags = flags;
    return node;
}

lStmt Branch::make(lOpr opr, std::string label, FlagsExpr flags) {
    internal_assert(!label.empty())
        << "Cannot make Branch with empty label";
    Branch *node = new Branch;
    node->opr = std::move(opr);
    node->label = std::move(label);
    node->flags = flags;
    return node;
}

lStmt Sequence::make(std::vector<lStmt> stmts) {
    internal_assert(!stmts.empty())
        << "Cannot make Sequence with empty stmts";
    Sequence *node = new Sequence;
    node->stmts = std::move(stmts);
    return node;
}

lStmt SpecialStmt::make(Kind kind) {
    SpecialStmt *node = new SpecialStmt;
    node->kind = std::move(kind);
    return node;
}

lStmt Label::make(std::string str) {
    Label * node = new Label;
    node->label = std::move(str);
    return node;
}

lStmt RawStmt::make(std::string text) {
    RawStmt *node = new RawStmt;
    node->text = std::move(text);
    return node;
}

}// namespace llir
} // namespace qpudsl
