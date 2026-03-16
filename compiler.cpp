#include <iostream>
#include "CompileToCIN.h"
#include "CompileToLLIR.h"
#include "Frontend.h"
#include "Printer.h"

using namespace qpudsl;

static void check(const char *label, const Expr &expr) {
    std::cout << label << "\n";
    CIN cin = compile_to_cin(expr, "Z");
    std::cout << cin << "\n";

    llir::lStmt kernel = compile_to_llir(cin);
    Printer p(std::cout);
    p.print(kernel);
    std::cout << "\n";
}

int main(int argc, char **argv) {
    // 1-D vectors
    TensorType VecType(Format::ordered({Level{"i"}}), dType::INT32);

    Expr A = Tensor::make(VecType, "A");
    Expr B = Tensor::make(VecType, "B");
    Expr C = Tensor::make(VecType, "C");
    Expr D = Tensor::make(VecType, "D");

    check("A+B",        A + B);
    check("A*B",        A * B);
    check("A+B*C",      A + B * C);
    check("A+B+C+D",    A + B + C + D);
    check("A*(B+C*D)",  A * (B + C * D));


    // 2-D matrices [R, I]: outermost dim r (rows), innermost dim i (columns)
    TensorType MatType(Format::ordered({Level{"r"}, Level{"i"}}), dType::INT32);

    Expr MA = Tensor::make(MatType, "MA");
    Expr MB = Tensor::make(MatType, "MB");
    Expr MC = Tensor::make(MatType, "MC");

    check("MA+MB (2D)",     MA + MB);
    check("MA+MB*MC (2D)",  MA + MB * MC);

    // 3-D tensors [B, R, I]
    TensorType TenType(Format::ordered({Level{"b"}, Level{"r"}, Level{"i"}}), dType::INT32);

    Expr TA = Tensor::make(TenType, "TA");
    Expr TB = Tensor::make(TenType, "TB");

    check("TA+TB (3D)", TA + TB);

    return 0;
}
