#include <iostream>
#include "CompileToCIN.h"
#include "CompileToLLIR.h"
#include "Parser.h"
#include "Printer.h"

using namespace qpudsl;

int main(int argc, char **argv) {
    if (argc < 2) {
        std::cerr << "usage: dsl <file.dsl>\n";
        return 1;
    }

    ParsedProgram prog = parse_dsl_file(argv[1]);

    CIN cin = compile_to_cin(prog.expr, "Z");
    std::cout << cin << "\n";

    llir::lStmt kernel = compile_to_llir(cin);
    Printer p(std::cout);
    p.print(kernel);
    std::cout << "\n";

    return 0;
}
