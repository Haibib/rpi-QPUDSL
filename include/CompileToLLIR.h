#pragma once

#include "CIN.h"
#include "llir/LLIR.h"

namespace qpudsl {

llir::lStmt compile_to_llir(const CIN &cin);

} // namespace qpudsl
