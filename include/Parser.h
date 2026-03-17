#pragma once

#include "Frontend.h"
#include "Type.h"

#include <cstdint>
#include <string>
#include <vector>
#include <utility>

namespace qpudsl {

struct ParsedTensorDecl {
    std::string          name;
    std::vector<int64_t> dims;
    dType                dtype;
};

struct ParsedScalarDecl {
    std::string name;
    int64_t     value;
    dType       dtype;
};


struct ParsedSliceRef {
    std::string                             base_name; 
    std::string                             gen_name; 
    std::vector<std::pair<int64_t,int64_t>> slices;   
};

struct ParsedProgram {
    std::vector<ParsedTensorDecl> tensors;
    std::vector<ParsedScalarDecl> scalars;
    std::vector<ParsedSliceRef>   slice_refs; 
    Expr                          expr;
};

ParsedProgram parse_dsl(const std::string &src);

ParsedProgram parse_dsl_file(const std::string &path);

} // namespace qpudsl
