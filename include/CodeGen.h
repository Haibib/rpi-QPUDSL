#pragma once

#include "CompileToLLIR.h"
#include "Parser.h"

#include <string>

namespace qpudsl {

void generate_caller_code(const ParsedProgram &prog,
                          const KernelInfo    &info,
                          const std::string   &stem,
                          const std::string   &out_dir);

} // namespace qpudsl
