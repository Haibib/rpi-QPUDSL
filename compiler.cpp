#include <stdio.h>
#include "CompileToCIN.h"
#include <iostream>
using namespace qpudsl;

int main(int argc, char **argv) {

    TensorType MatrixType(qpudsl::Format::ordered({qpudsl::Level{"i"}, qpudsl::Level{"j"}}), qpudsl::dType::INT32);
    
    Expr A = Tensor::make(MatrixType, "A");
    Expr B = Tensor::make(MatrixType, "B");
    Expr C = Tensor::make(MatrixType, "C");
    Expr D = Tensor::make(MatrixType, "D");

    // Define the computation
    Expr E = A + B + C + D;

    std::cout<<compile_to_cin(E);

    return 0;
}