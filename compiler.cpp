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

    Expr A_sliced = slice(A, {SliceRange{0, 48, 96}});
    Expr B_sliced = slice(B, {SliceRange{48, 48, 96}});
    check("slice(A,0:48) + slice(B,48:96)", A_sliced + B_sliced);

    // 2-D matrices [R, I]: outermost dim r (rows), innermost dim i (columns)
    TensorType MatType(Format::ordered({Level{"r"}, Level{"i"}}), dType::INT32);

    Expr MA = Tensor::make(MatType, "MA");
    Expr MB = Tensor::make(MatType, "MB");
    Expr MC = Tensor::make(MatType, "MC");

    check("MA+MB (2D)",     MA + MB);
    check("MA+MB*MC (2D)",  MA + MB * MC);


    Expr MA_sliced = slice(MA, {SliceRange{0, 5, 16}, SliceRange{0, 48, 48}});
    Expr MB_sliced2 = slice(MB, {SliceRange{0, 5, 16}, SliceRange{0, 48, 48}});
    check("slice(MA,r=0:5,i=0:48) + slice(MB,r=0:5,i=0:48) (2D)", MA_sliced + MB_sliced2);


    TensorType WideMatType(Format::ordered({Level{"r"}, Level{"i"}}), dType::INT32);

    Expr WA = Tensor::make(WideMatType, "WA");
    Expr WB = Tensor::make(WideMatType, "WB");


    Expr WA_s = slice(WA, {SliceRange{0, 7, 8}, SliceRange{0, 48, 48}});
    Expr WB_s = slice(WB, {SliceRange{0, 7, 8}, SliceRange{0, 48, 48}});
    check("WA+WB (2D, 7-rows 48-wide)", WA_s + WB_s);

    // 3-D tensors [B, R, I]
    TensorType TenType(Format::ordered({Level{"b"}, Level{"r"}, Level{"i"}}), dType::INT32);

    Expr TA = Tensor::make(TenType, "TA");
    Expr TB = Tensor::make(TenType, "TB");

    check("TA+TB (3D)", TA + TB);

    // 3-D sliced: 5 batches , 3 rows, 48 cols 
    Expr TA_sliced = slice(TA, {SliceRange{0, 5, 8}, SliceRange{0, 3, 4}, SliceRange{0, 48, 48}});
    Expr TB_sliced = slice(TB, {SliceRange{0, 5, 8}, SliceRange{0, 3, 4}, SliceRange{0, 48, 48}});
    check("slice(TA,b=0:5,r=0:3,i=0:48) + slice(TB,...) (3D)", TA_sliced + TB_sliced);

    return 0;
}
