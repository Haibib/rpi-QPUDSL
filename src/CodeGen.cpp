#include "CodeGen.h"

#include <cctype>
#include <fstream>
#include <sstream>
#include <stdexcept>

namespace qpudsl {

namespace {

std::string format(const std::string &str) {
    std::string s;
    for (char c : str) s += (c == '-') ? '_' : c;
    return s;
}

std::string format_upper(const std::string &str) {
    std::string s = format(str);
    for (char &c : s) c = (char)std::toupper((unsigned char)c);
    return s;
}

std::string format2(const std::string &str) {
    std::string s;
    bool cap = true;
    for (char c : str) {
        if (c == '-' || c == '_') { cap = true; continue; }
        s += cap ? (char)std::toupper((unsigned char)c) : c;
        cap = false;
    }
    return s;
}


std::string macro(const std::string &str, const std::string &name) {
    std::string upper;
    for (char c : name) upper += (char)std::toupper((unsigned char)c);
    return str+"_"+upper;
}

std::string dim_product_expr(const std::vector<int64_t> &dims) {
    if (dims.empty()) return "1";
    if (dims.size() == 1) return std::to_string(dims[0]);
    std::string s = "(";
    for (int i = 0; i < (int)dims.size(); ++i) {
        if (i > 0) s+="*";
        s += std::to_string(dims[i]);
    }
    s+=")";
    return s;
}

int64_t dims_product(const std::vector<int64_t> &dims) {
    int64_t p = 1;
    for (auto d : dims) p *= d;
    return p;
}

}

void generate_caller_code(const ParsedProgram &prog,
                          const KernelInfo    &info,
                          const std::string   &filename,
                          const std::string   &out_dir) {
    const std::string name   = format(filename);
    const std::string NAME   = format_upper(filename);
    const std::string Name    = format2(filename);
    const std::string struct_t = Name + "GPU";

    int N = (int)info.tensors.size();
    int D = info.D;
    int S = (int)info.scalars.size();
    int num_unifs = N + 1 + (N + 1) * D + 2 * D + S + 1;

    
    const ParsedTensorDecl *ref_decl = prog.tensors.empty() ? nullptr : &prog.tensors[0];
    struct TensorSlot {
        std::string name;
        std::vector<int64_t> dims;
        int64_t n_elems;
    };
    std::vector<TensorSlot> slots;
    for (const auto &td : prog.tensors)
        slots.push_back({td.name, td.dims, dims_product(td.dims)});

    std::vector<int64_t> offsets;
    int64_t running = 0;
    for (const auto &sl : slots) {
        offsets.push_back(running);
        running += sl.n_elems;
    }
    int64_t z_off     = running;
    int64_t z_n       = slots.empty() ? 0 : slots[0].n_elems;
    int64_t total_buf = z_off + z_n;

    std::ostringstream hdr;

    hdr << "#pragma once\n"
        << "#include \"" << name << "_shader.h\"\n"
        << "#include \"rpi.h\"\n"
        << "#include <stdint.h>\n\n"
        << "#ifndef GPU_MEM_FLG\n"
        << "#define GPU_MEM_FLG 0xC\n"
        << "#endif\n"
        << "#ifndef GPU_BASE\n"
        << "#define GPU_BASE    0x40000000\n"
        << "#endif\n\n";

    
    for (const auto &sl : slots)
        hdr << "#define " << macro(NAME, sl.name) << "_N    "
            << dim_product_expr(sl.dims) << "\n";
    hdr << "#define " << NAME << "_Z_N      "
        << (slots.empty() ? "0" : dim_product_expr(slots[0].dims)) << "\n\n";

    
    for (int ti = 0; ti < (int)slots.size(); ++ti) {
        hdr << "#define " << macro(NAME, slots[ti].name) << "_OFF  ";
        if (offsets[ti] == 0) {
            hdr << "0";
        } else {
            std::string expr;
            for (int j = 0; j < ti; ++j) {
                if (j > 0) expr += " + ";
                expr += macro(NAME, slots[j].name) + "_N";
            }
            hdr << "(" << expr << ")";
        }
        hdr << "\n";
    }
    
    {
        std::string expr;
        for (int ti = 0; ti < (int)slots.size(); ++ti) {
            if (ti > 0) expr += " + ";
            expr += macro(NAME, slots[ti].name) + "_N";
        }
        hdr << "#define " << NAME << "_Z_OFF    "
            << (slots.empty() ? "0" : "(" + expr + ")") << "\n";
    }
    
    {
        std::string expr;
        for (int ti = 0; ti < (int)slots.size(); ++ti) {
            if (ti > 0) expr += " + ";
            expr += macro(NAME, slots[ti].name) + "_N";
        }
        if (!slots.empty()) expr += " + ";
        expr += NAME + "_Z_N";
        hdr << "#define " << NAME << "_BUF_N    (" << expr << ")\n\n";
    }

    hdr << "#define " << NAME << "_NUM_UNIFS " << num_unifs << "\n\n";

    hdr << "uint32_t *" << name << "_init(void);\n\n";

    hdr << "uint32_t " << name << "_kernel(";
    for (int i = 0; i < N; ++i) hdr << "void *in" << i << ", ";
    hdr << "void *out";
    for (int s = 0; s < S; ++s)
        hdr << ", uint32_t " << info.scalars[s];
    hdr << ");\n\n";

    hdr << "void " << name << "_release(void);\n";

    std::ostringstream src;

    src << "#include <stddef.h>\n"
        << "#include <string.h>\n"
        << "#include \"" <<name << "_qpu.h\"\n"
        << "#include \"mailbox.h\"\n"
        << "#include \"" << name << "_shader.h\"\n\n";

    // Private GPU struct
    src << "struct " << struct_t << " {\n"
        << "    uint32_t buf[" << NAME << "_BUF_N];\n"
        << "    uint32_t code[sizeof(" << name<< "_shader) / sizeof(uint32_t)];\n"
        << "    uint32_t unif[8][" << NAME<< "_NUM_UNIFS];\n"
        << "    uint32_t unif_ptr[8];\n"
        << "    uint32_t mail[2];\n"
        << "    uint32_t handle;\n"
        << "};\n\n";

    src << "static volatile struct " << struct_t << " *_gpu = NULL;\n\n";

    // _init
    src << "uint32_t *" << name<< "_init(void) {\n"
        << "    uint32_t handle, vc;\n"
        << "    volatile struct " << struct_t << " *ptr;\n\n"
        << "    if (qpu_enable(1))\n"
        << "        panic(\"" << name << "_init: failed to enable QPU\");\n\n"
        << "    handle = mem_alloc(sizeof(struct " << struct_t << "), 4096, GPU_MEM_FLG);\n"
        << "    if (!handle) {\n"
        << "        qpu_enable(0);\n"
        << "        panic(\"" << name << "_init: failed to allocate GPU memory\");\n"
        << "    }\n\n"
        << "    vc  = mem_lock(handle);\n"
        << "    ptr = (volatile struct " << struct_t << " *)(vc - GPU_BASE);\n"
        << "    if (!ptr) {\n"
        << "        mem_free(handle);\n"
        << "        mem_unlock(handle);\n"
        << "        qpu_enable(0);\n"
        << "        panic(\"" << name << "_init: failed to map GPU memory\");\n"
        << "    }\n\n"
        << "    ptr->handle  = handle;\n"
        << "    ptr->mail[0] = vc + offsetof(struct " << struct_t << ", code);\n"
        << "    ptr->mail[1] = vc + offsetof(struct " << struct_t << ", unif);\n"
        << "    memcpy((void *)ptr->code, " << name << "_shader, sizeof(ptr->code));\n\n"
        << "    _gpu = ptr;\n"
        << "    return (uint32_t *)_gpu->buf;\n"
        << "}\n\n";

    // _kernel
    src << "uint32_t " << name << "_kernel(";
    for (int i = 0; i < N; ++i) src << "void *in" << i << ", ";
    src << "void *out";
    for (int s = 0; s < S; ++s)
        src << ", uint32_t " << info.scalars[s];
    src << ") {\n";

    src << "    uint32_t vc_base = _gpu->mail[0] - offsetof(struct "
        << struct_t << ", code);\n";
    if (N > 0) {
        src << "    void *in_ptrs[" << N << "] = {";
        for (int i = 0; i < N; ++i) {
            if (i > 0) src << ", ";
            src << "in" << i;
        }
        src << "};\n";
    }
    src << "\n"
        << "    for (uint32_t q = 0; q < 8; q++) {\n"
        << "        int u = 0;\n";
    for (int i = 0; i < N; ++i)
        src << "        _gpu->unif[q][u++] = vc_base + ((uint8_t *)in_ptrs[" << i
            << "] - (uint8_t *)_gpu);\n";
    src << "        _gpu->unif[q][u++] = vc_base + ((uint8_t *)out - (uint8_t *)_gpu);\n"
        << "        for (int i = 0; i < " << (N + 1) * D << "; i++) _gpu->unif[q][u++] = 0;\n";
    for (int d = 0; d < D; ++d) {
        int64_t ds = ref_decl ? ref_decl->dims[D - 1 - d] : 0;
        src << "        _gpu->unif[q][u++] = " << ds << ";\n";
    }
    for (int d = 0; d < D; ++d) {
        int64_t ds = ref_decl ? ref_decl->dims[D - 1 - d] : 0;
        src << "        _gpu->unif[q][u++] = " << ds << ";\n";
    }
    for (int s = 0; s < S; ++s)
        src << "        _gpu->unif[q][u++] = " << info.scalars[s] << ";\n";
    src << "        _gpu->unif[q][u++] = q;\n"
        << "        _gpu->unif_ptr[q] = _gpu->mail[1]"
        << " + q * " << name << "_NUM_UNIFS * sizeof(uint32_t);\n"
        << "    }\n\n"
        << "    return gpu_fft_base_exec_direct(_gpu->mail[0],\n"
        << "                                    (uint32_t *)_gpu->unif_ptr, 8);\n"
        << "}\n\n";

    // _release
    src << "void " << name << "_release(void) {\n"
        << "    if (!_gpu) return;\n"
        << "    uint32_t handle = _gpu->handle;\n"
        << "    mem_unlock(handle);\n"
        << "    mem_free(handle);\n"
        << "    qpu_enable(0);\n"
        << "    _gpu = NULL;\n"
        << "}\n";

    std::string dir = out_dir;
    if (!dir.empty() && dir.back() != '/') dir += '/';

    auto write_file = [](const std::string &path, const std::string &content) {
        std::ofstream f(path);
        if (!f) throw std::runtime_error("failed to write " + path);
        f << content;
    };

    write_file(dir + filename + "_qpu.h", hdr.str());
    write_file(dir + filename + "_qpu.c", src.str());
}

} // namespace qpudsl
