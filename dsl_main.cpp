#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>

#include "CodeGen.h"
#include "CompileToCIN.h"
#include "CompileToLLIR.h"
#include "Parser.h"
#include "Printer.h"

using namespace qpudsl;
namespace fs = std::filesystem;

int main(int argc, char **argv) {
    if (argc < 2) {
        std::cerr << "usage: dsl <file.appil> [output_dir]\n";
        return 1;
    }

    fs::path input_path(argv[1]);
    std::string filename = input_path.stem().string();

    fs::path out_dir;
    if (argc >= 3) {
        out_dir = fs::path(argv[2]);
    } else {
        out_dir = input_path.parent_path();
        if (out_dir.empty()) out_dir = ".";
    }

    ParsedProgram prog = parse_dsl_file(argv[1]);

    CIN cin = compile_to_cin(prog.expr, prog.out_name.empty() ? "Z" : prog.out_name);

    llir::lStmt kernel = compile_to_llir(cin);

    fs::path qasm_path = out_dir / (filename + ".qasm");
    std::ofstream qf(qasm_path);
    if (!qf) {
        std::cerr << "error: cannot open " << qasm_path << "\n";
        return 1;
    }
    Printer p(qf);
    p.print(kernel);
    qf << "\n";
    std::cout << "wrote " << qasm_path << "\n";

    KernelInfo info = analyze_kernel(cin);
    generate_caller_code(prog, info, filename, out_dir.string());

    return 0;
}
