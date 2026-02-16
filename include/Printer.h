#pragma once

#include "Visitor.h"
#include "Type.h"

#include <iostream>

namespace qpudsl {



std::ostream &operator<<(std::ostream &os, const Level &lvl);
std::ostream &operator<<(std::ostream &os, const Format &format);



class Printer : public Visitor {

};

} // namespace qpudsl