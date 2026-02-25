#pragma once

#include "IRFwdDecl.h"
#include "CIN.h" 
#include "Frontend.h"
#include "Printer.h"
#include "Error.h"
#include <string>

namespace qpudsl {

CIN compile_to_cin(const Expr &expr, std::string out = "Z");

} // namespace qpudsl
