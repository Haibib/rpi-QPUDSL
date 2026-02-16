#pragma once

#include "IRFwdDecl.h"

namespace qpudsl {

struct Visitor {
    
};

#define RESTRICT_VISITOR(IRNODE)                                               \
    void visit(const IRNODE *) final {                                         \
        internal_error << "Restricted Visitor class does not handle: "         \
                       << typeid(IRNODE).name();                               \
    }

} // namespace qpudsl
