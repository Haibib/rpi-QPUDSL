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

    // 1-D sliced: A[0:256] + B[256:512]  (D=1: same dim is both innermost and outermost,
    // so size must be divisible by 32*8=256: innermost body=32 elems, outermost=8 QPUs)
    Expr A_sliced = slice(A, {SliceRange{0, 256, 512}});
    Expr B_sliced = slice(B, {SliceRange{256, 256, 512}});
    check("slice(A,0:256) + slice(B,256:512)", A_sliced + B_sliced);

    // 2-D matrices [R, I]: outermost dim r (rows), innermost dim i (columns)
    TensorType MatType(Format::ordered({Level{"r"}, Level{"i"}}), dType::INT32);

    Expr MA = Tensor::make(MatType, "MA");
    Expr MB = Tensor::make(MatType, "MB");
    Expr MC = Tensor::make(MatType, "MC");

    check("MA+MB (2D)",     MA + MB);
    check("MA+MB*MC (2D)",  MA + MB * MC);

    // 2-D sliced: rows 0..7, all 32 columns  (outermost size=8 div by 8; innermost size=32 div by 32)
    Expr MA_sliced = slice(MA, {SliceRange{0, 8, 16}, SliceRange{0, 32, 32}});
    Expr MB_sliced2 = slice(MB, {SliceRange{0, 8, 16}, SliceRange{0, 32, 32}});
    check("slice(MA,r=0:8,i=0:32) + slice(MB,r=0:8,i=0:32) (2D)", MA_sliced + MB_sliced2);

    // 2-D matrices with wide innermost dimension: 64 columns, 4 tiles per row (64/16=4)
    TensorType WideMatType(Format::ordered({Level{"r"}, Level{"i"}}), dType::INT32);

    Expr WA = Tensor::make(WideMatType, "WA");
    Expr WB = Tensor::make(WideMatType, "WB");

    // slice_size[i=0]=64 cols (innermost, div by 32); slice_size[r=1]=8 rows (outermost, div by 8)
    Expr WA_s = slice(WA, {SliceRange{0, 8, 8}, SliceRange{0, 64, 64}});
    Expr WB_s = slice(WB, {SliceRange{0, 8, 8}, SliceRange{0, 64, 64}});
    check("WA+WB (2D, 64-wide)", WA_s + WB_s);

    // 3-D tensors [B, R, I]: outermost dim b (batch), middle dim r (row), innermost dim i (column)
    TensorType TenType(Format::ordered({Level{"b"}, Level{"r"}, Level{"i"}}), dType::INT32);

    Expr TA = Tensor::make(TenType, "TA");
    Expr TB = Tensor::make(TenType, "TB");

    check("TA+TB (3D)", TA + TB);

    // 3-D sliced: batches 0..7, rows 0..3, cols 0..31  (outermost size=8 div by 8; innermost size=32 div by 32)
    Expr TA_sliced = slice(TA, {SliceRange{0, 8, 8}, SliceRange{0, 4, 4}, SliceRange{0, 32, 32}});
    Expr TB_sliced = slice(TB, {SliceRange{0, 8, 8}, SliceRange{0, 4, 4}, SliceRange{0, 32, 32}});
    check("slice(TA,b=0:8,r=0:4,i=0:32) + slice(TB,...) (3D)", TA_sliced + TB_sliced);

    return 0;
}
