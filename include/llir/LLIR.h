#pragma once

#include "IRHandle.h"
#include "IRNode.h"
#include "IntrusivePtr.h"
#include "Visitor.h"

#include <cstdint>
#include <variant>
#include <vector>

namespace qpudsl {
namespace llir {

struct lRegType;
struct lOpr;
struct lStmt;

// Types (for declarations)
// TODO: make this complete
enum class lRegTypeEnum { Accumulator, MemoryA, MemoryB, Special };

// Expressions
enum class lOprEnum {
    Reg,
    Const,
    Macro,
};

// Statements
enum class lStmtEnum {
    Mov,
    Shl,
    Add,
    Sub,
    Mul,
    Branch,
    Sequence,
    SpecialStmt,
    Label
};
    
enum class FlagsExpr {
        None,
        SetF,
        AnyNZ
    };

using IRlRegTypeNode = IRNode<lRegType, lRegTypeEnum>;

using IRlOprNode = IRNode<lOpr, lOprEnum>;

using IRlStmtNode = IRNode<lStmt, lStmtEnum>;

// This is necessary to get mutate() to work properly...
struct BaselRegTypeNode : public IRlRegTypeNode {
    BaselRegTypeNode(lRegTypeEnum t) : IRlRegTypeNode(t) {}
    // virtual lStmt mutate_lStmt(Mutator *m) const = 0;
};

template <typename T>
struct lRegTypeNode : public BaselRegTypeNode {
    void accept(Visitor *v) const override { return v->visit((const T *)this); }
    // lRegType mutate_lRegType(Mutator *m) const override;
    lRegTypeNode() : BaselRegTypeNode(T::node_type) {}
    ~lRegTypeNode() override = default;
};

struct lRegType : public IRHandle<IRlRegTypeNode> {
    /** Make an undefined lRegType */
    lRegType() = default;

    /** Make a lRegType from a concrete lRegType node pointer (e.g. Int_t) */
    lRegType(const IRlRegTypeNode *n) : IRHandle<IRlRegTypeNode>(n) {}

    /** Override get() to return a BaselRegTypeNode * instead of an IRNode.
     *  This is necessary to get mutate() to work properly. **/
    const BaselRegTypeNode *get() const { return (const BaselRegTypeNode *)ptr; }

    // TODO: implement copy/move semantics!
};

// template <typename T>
// lType lTypeNode<T>::mutate_lType(Mutator *m) const {
//     return m->visit((const T *)this);
// }

struct BaselOprNode : public IRlOprNode {
    BaselOprNode(lOprEnum t) : IRlOprNode(t) {}
    // virtual lOpr mutate_lOpr(Mutator *m) const = 0;
};

template <typename T>
struct lOprNode : public BaselOprNode {
    void accept(Visitor *v) const override { return v->visit((const T *)this); }
    // lExpr mutate_lExpr(Mutator *m) const override;
    lOprNode() : BaselOprNode(T::node_type) {}
    ~lOprNode() override = default;
};

struct lOpr : public IRHandle<IRlOprNode> {
    /** Make an undefined lOpr */
    lOpr() = default;

    /** Make a lOpr from a concrete lOpr node pointer (e.g. lBinOp) */
    lOpr(const IRlOprNode *n) : IRHandle<IRlOprNode>(n) {}

    /** Override get() to return a BaselOprNode * instead of an IRNode.
     *  This is necessary to get mutate() to work properly. **/
    const BaselOprNode *get() const { return (const BaselOprNode *)ptr; }

    // TODO: implement copy/move semantics!

    // Overloads a[b] operation.
    lOpr operator[](const lOpr &idx);
};

// template <typename T>
// lExpr lExprNode<T>::mutate_lExpr(Mutator *m) const {
//     return m->visit((const T *)this);
// }

struct BaselStmtNode : public IRlStmtNode {
    BaselStmtNode(lStmtEnum t) : IRlStmtNode(t) {}
    // virtual lStmt mutate_lStmt(Mutator *m) const = 0;
};



template <typename T>
struct lStmtNode : public BaselStmtNode {
    FlagsExpr flags;

    void accept(Visitor *v) const override { return v->visit((const T *)this); }
    // lStmt mutate_lStmt(Mutator *m) const override;
    lStmtNode() : BaselStmtNode(T::node_type) {}
    ~lStmtNode() override = default;
};

struct lStmt : public IRHandle<IRlStmtNode> {
    /** Make an undefined lStmt */
    lStmt() = default;

    /** Make an lStmt from a concrete lStmt node pointer (e.g. While) */
    lStmt(const IRlStmtNode *n) : IRHandle<IRlStmtNode>(n) {}

    /** Override get() to return a BaselStmtNode * instead of an IRNode.
     *  This is necessary to get mutate() to work properly. **/
    const BaselStmtNode *get() const { return (const BaselStmtNode *)ptr; }

    // TODO: implement copy/move semantics!
};

// template <typename T>
// lStmt lStmtNode<T>::mutate_lStmt(Mutator *m) const {
//     return m->visit((const T *)this);
// }

struct Accumulator : lRegTypeNode<Accumulator> {
    static lRegType make();

    static const lRegTypeEnum node_type = lRegTypeEnum::Accumulator;
};

struct MemoryA : lRegTypeNode<MemoryA> {
    static lRegType make();

    static const lRegTypeEnum node_type = lRegTypeEnum::MemoryA;
};

struct MemoryB : lRegTypeNode<MemoryB> {
    static lRegType make();

    static const lRegTypeEnum node_type = lRegTypeEnum::MemoryB;
};

struct Special : lRegTypeNode<Special> {
    enum Kind {
        UNIF,
        VR_SETUP,
        VR_ADDR,
        VR_WAIT,
        VPM,
        EMPTY,
        INTERRUPT
    };

    Kind kind;
    static lRegType make(Kind kind);

    static const lRegTypeEnum node_type = lRegTypeEnum::Special;
};


struct Reg : lOprNode<Reg> {
    lRegType type;
    int num;

    static lOpr make(lRegType type, int num);

    static const lOprEnum node_type = lOprEnum::Reg;
};


struct Const : lOprNode<Const> {
    int64_t value;

    static lOpr make(int64_t value);

    static const lOprEnum node_type = lOprEnum::Const;
};

struct Macro : lOprNode<Macro> {
    std::string str;

    static lOpr make(std::string str);

    static const lOprEnum node_type = lOprEnum::Macro;
};


struct Mov : lStmtNode<Mov> {
    lOpr dst, src;

    static lStmt make(lOpr dst, lOpr src, FlagsExpr flags=FlagsExpr::None);

    static const lStmtEnum node_type = lStmtEnum::Mov;
};

struct Shl : lStmtNode<Shl> {
    lOpr dst, src, shift;

    static lStmt make(lOpr dst, lOpr src, lOpr shift, FlagsExpr flags=FlagsExpr::None);

    static const lStmtEnum node_type = lStmtEnum::Shl;
};

struct Add : lStmtNode<Add> {
    lOpr dst, src0, src1;

    static lStmt make(lOpr dst, lOpr src0, lOpr src1, FlagsExpr flags=FlagsExpr::None);

    static const lStmtEnum node_type = lStmtEnum::Add;
};

struct Sub : lStmtNode<Sub> {
    lOpr dst, src0, src1;

    static lStmt make(lOpr dst, lOpr src0, lOpr src1, FlagsExpr flags=FlagsExpr::None);

    static const lStmtEnum node_type = lStmtEnum::Sub;
};

struct Mul : lStmtNode<Mul> {
    lOpr dst, src0, src1;

    static lStmt make(lOpr dst, lOpr src0, lOpr src1, FlagsExpr flags=FlagsExpr::None);

    static const lStmtEnum node_type = lStmtEnum::Mul;
};

struct Branch : lStmtNode<Branch> {
    lOpr opr;
    std::string label;

    static lStmt make(lOpr opr, std::string label, FlagsExpr flags=FlagsExpr::None);

    static const lStmtEnum node_type = lStmtEnum::Branch;
};

struct Sequence : lStmtNode<Sequence> {
    std::vector<lStmt> stmts;

    static lStmt make(std::vector<lStmt> stmts);

    static const lStmtEnum node_type = lStmtEnum::Sequence;
};

struct Label : lStmtNode<Label> {
    std::string label;
    static lStmt make(std::string label);
    static const lStmtEnum node_type = lStmtEnum::Label;
};

struct SpecialStmt : lStmtNode<SpecialStmt> {
    enum Kind {
        NOP,
        THREND,
    };

    Kind kind;

    static lStmt make(Kind kind);

    static const lStmtEnum node_type = lStmtEnum::SpecialStmt;
};

} // namespace llir

template <>
inline RefCount &
ref_count<llir::IRlRegTypeNode>(const llir::IRlRegTypeNode *t) noexcept {
    return t->ref_count;
}

template <>
inline void destroy<llir::IRlRegTypeNode>(const llir::IRlRegTypeNode *t) {
    delete t;
}

template <>
inline RefCount &
ref_count<llir::IRlOprNode>(const llir::IRlOprNode *t) noexcept {
    return t->ref_count;
}

template <>
inline void destroy<llir::IRlOprNode>(const llir::IRlOprNode *t) {
    delete t;
}

template <>
inline RefCount &
ref_count<llir::IRlStmtNode>(const llir::IRlStmtNode *t) noexcept {
    return t->ref_count;
}

template <>
inline void destroy<llir::IRlStmtNode>(const llir::IRlStmtNode *t) {
    delete t;
}

} // namespace qpudsl
