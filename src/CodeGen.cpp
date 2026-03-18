#include "CodeGen.h"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <map>
#include <sstream>
#include <stdexcept>
#include <iostream>

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
    return str + "_" + upper;
}

std::string dim_product_expr(const std::vector<int64_t> &dims) {
    if (dims.empty()) return "1";
    if (dims.size() == 1) return std::to_string(dims[0]);
    std::string s = "(";
    for (int i = 0; i < (int)dims.size(); ++i) {
        if (i > 0) s += "*";
        s += std::to_string(dims[i]);
    }
    s += ")";
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
    const std::string name    = format(filename);
    const std::string NAME    = format_upper(filename);
    const std::string Name    = format2(filename);
    const std::string struct_t = Name + "GPU";

    int N = (int)info.tensors.size();
    int D = info.D;
    int S = (int)info.scalars.size();
    int num_unifs = N + 1 + (N + 1) * D + 2 * D + S + 1;


    std::map<std::string, const ParsedSliceRef *> slice_map;
    for (const auto &sr : prog.slice_refs) slice_map[sr.gen_name] = &sr;

    std::map<std::string, const ParsedTensorDecl *> tensor_map;
    for (const auto &td : prog.tensors) tensor_map[td.name] = &td;

    const ParsedTensorDecl *ref_decl = prog.tensors.empty() ? nullptr : &prog.tensors[0];
    if (!info.tensors.empty()) {
        auto it = slice_map.find(info.tensors[0].name);
        if (it != slice_map.end()) {
            auto jt = tensor_map.find(it->second->base_name);
            if (jt != tensor_map.end()) ref_decl = jt->second;
        }
    }

    const ParsedSliceRef *first_slice_ref = nullptr;
    if (!info.tensors.empty()) {
        auto it = slice_map.find(info.tensors[0].name);
        if (it != slice_map.end()) first_slice_ref = it->second;
    }

    std::vector<std::string> unique_bases;
    for (int i = 0; i < N; ++i) {
        std::string base = info.tensors[i].name;
        auto it = slice_map.find(base);
        if (it != slice_map.end()) base = it->second->base_name;
        if (std::find(unique_bases.begin(), unique_bases.end(), base) == unique_bases.end())
            unique_bases.push_back(base);
    }
    const std::string out_name_fmt = format(prog.out_name);
    bool out_in_bases = std::find(unique_bases.begin(), unique_bases.end(), prog.out_name) != unique_bases.end();

    struct TensorSlot {
        std::string name;
        std::vector<int64_t> dims;
        int64_t n_elems;
    };
    std::vector<TensorSlot> slots;
    for (const auto &td : prog.tensors)
        slots.push_back({td.name, td.dims, dims_product(td.dims)});


    bool out_is_declared = tensor_map.count(prog.out_name) > 0;
    int64_t out_n_elems = 0;
    if (!out_is_declared) {
        if (first_slice_ref && !first_slice_ref->slices.empty()) {
            out_n_elems = 1;
            for (const auto &sl : first_slice_ref->slices)
                out_n_elems *= (sl.second - sl.first);
        } else if (!slots.empty()) {
            out_n_elems = slots[0].n_elems;
        }
    }

    std::vector<int64_t> offsets;
    int64_t running = 0;
    for (const auto &sl : slots) {
        offsets.push_back(running);
        running += sl.n_elems;
    }
    int64_t out_off = out_is_declared ? offsets[tensor_map.at(prog.out_name) - tensor_map.begin()->second] : running;

    int out_slot_idx = -1;
    for (int i = 0; i < (int)slots.size(); ++i)
        if (slots[i].name == prog.out_name) { out_slot_idx = i; break; }
    if (out_slot_idx >= 0)
        out_off = offsets[out_slot_idx];
    else
        out_off = running;

    int64_t total_buf = out_is_declared ? running : running + out_n_elems;

    std::ostringstream hdr;

    hdr << "#pragma once\n"
        << "#include \"" << filename << ".h\"\n"
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

    if (!out_is_declared) {
        std::vector<int64_t> out_dims;
        if (first_slice_ref && !first_slice_ref->slices.empty())
            for (const auto &sl : first_slice_ref->slices)
                out_dims.push_back(sl.second - sl.first);
        else if (!slots.empty())
            out_dims = slots[0].dims;
        hdr << "#define " << macro(NAME, prog.out_name) << "_N      "
            << dim_product_expr(out_dims) << "\n";
    }
    hdr << "\n";

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

    if (out_slot_idx < 0) {
        hdr << "#define " << macro(NAME, prog.out_name) << "_OFF  ";
        if (slots.empty()) {
            hdr << "0";
        } else {
            std::string expr;
            for (int j = 0; j < (int)slots.size(); ++j) {
                if (j > 0) expr += " + ";
                expr += macro(NAME, slots[j].name) + "_N";
            }
            hdr << "(" << expr << ")";
        }
        hdr << "\n";
    }

    {
        std::string expr;
        for (const auto &sl : slots) {
            if (!expr.empty()) expr += " + ";
            expr += macro(NAME, sl.name) + "_N";
        }
        if (!out_is_declared) {
            if (!expr.empty()) expr += " + ";
            expr += macro(NAME, prog.out_name) + "_N";
        }
        hdr << "#define " << NAME << "_BUF_N    (" << expr << ")\n\n";
    }

    hdr << "#define " << NAME << "_NUM_UNIFS " << num_unifs << "\n\n";

    hdr << "uint32_t *" << name << "_init(void);\n\n";

    hdr << "uint32_t " << name << "_kernel(";
    for (const auto &b : unique_bases)
        hdr << "void *" << format(b) << ", ";
    if (!out_in_bases)
        hdr << "void *" << out_name_fmt;
    else {
        // remove trailing ", "
        std::string s = hdr.str();
        hdr.str(""); hdr.clear();
        hdr << s.substr(0, s.size() - 2);
    }
    for (int s = 0; s < S; ++s)
        hdr << ", uint32_t " << info.scalars[s];
    hdr << ");\n\n";

    hdr << "void " << name << "_release(void);\n";

    std::ostringstream src;

    src << "#include <stddef.h>\n"
        << "#include <string.h>\n"
        << "#include \"" << filename << "-qpu.h\"\n"
        << "#include \"mailbox.h\"\n"
        << "#include \"" << filename << ".h\"\n\n";

    src << "struct " << struct_t << " {\n"
        << "    uint32_t buf[" << NAME << "_BUF_N];\n"
        << "    uint32_t code[sizeof(" << name << ") / sizeof(uint32_t)];\n"
        << "    uint32_t unif[8][" << NAME << "_NUM_UNIFS];\n"
        << "    uint32_t unif_ptr[8];\n"
        << "    uint32_t mail[2];\n"
        << "    uint32_t handle;\n"
        << "};\n\n";

    src << "static volatile struct " << struct_t << " *_gpu = NULL;\n\n";

    src << "uint32_t *" << name << "_init(void) {\n"
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
        << "    memcpy((void *)ptr->code, " << name << ", sizeof(ptr->code));\n\n"
        << "    _gpu = ptr;\n"
        << "    return (uint32_t *)_gpu->buf;\n"
        << "}\n\n";

    src << "uint32_t " << name << "_kernel(";
    for (const auto &b : unique_bases)
        src << "void *" << format(b) << ", ";
    if (!out_in_bases)
        src << "void *" << out_name_fmt;
    else {
        std::string s = src.str();
        src.str(""); src.clear();
        src << s.substr(0, s.size() - 2);
    }
    for (int s = 0; s < S; ++s)
        src << ", uint32_t " << info.scalars[s];
    src << ") {\n";

    src << "    uint32_t vc_base = _gpu->mail[0] - offsetof(struct "
        << struct_t << ", code);\n\n"
        << "    for (uint32_t q = 0; q < 8; q++) {\n"
        << "        int u = 0;\n";

    for (int i = 0; i < N; ++i) {
        std::string base = info.tensors[i].name;
        auto it = slice_map.find(base);
        if (it != slice_map.end()) base = it->second->base_name;
        src << "        _gpu->unif[q][u++] = vc_base + ((uint8_t *)" << format(base)
            << " - (uint8_t *)_gpu);\n";
    }

    src << "        _gpu->unif[q][u++] = vc_base + ((uint8_t *)" << out_name_fmt
        << " - (uint8_t *)_gpu);\n";

    // slice_start[i][d] (innermost-first) for N inputs then 1 output
    for (int i = 0; i < N; ++i) {
        const ParsedSliceRef *sr = nullptr;
        auto it = slice_map.find(info.tensors[i].name);
        if (it != slice_map.end()) sr = it->second;
        for (int d = 0; d < D; ++d) {
            // d is innermost-first; DSL dim index = D-1-d
            int64_t start = 0;
            if (sr && D - 1 - d < (int)sr->slices.size())
                start = sr->slices[D - 1 - d].first;
            src << "        _gpu->unif[q][u++] = " << start << ";\n";
        }
    }
    // output slice_start
    for (int d = 0; d < D; ++d) {
        int64_t start = 0;
        if (!prog.out_slices.empty() && D - 1 - d < (int)prog.out_slices.size())
            start = prog.out_slices[D - 1 - d].first;
        src << "        _gpu->unif[q][u++] = " << start << ";\n";
    }

    // dim_size[d]
    for (int d = 0; d < D; ++d) {
        int64_t ds = ref_decl ? ref_decl->dims[D - 1 - d] : 0;
        src << "        _gpu->unif[q][u++] = " << ds << ";\n";
    }

    // slice_size[d]
    for (int d = 0; d < D; ++d) {
        int64_t sz = ref_decl ? ref_decl->dims[D - 1 - d] : 0;
        if (first_slice_ref && D - 1 - d < (int)first_slice_ref->slices.size())
            sz = first_slice_ref->slices[D - 1 - d].second - first_slice_ref->slices[D - 1 - d].first;
        src << "        _gpu->unif[q][u++] = " << sz << ";\n";
    }

    for (int s = 0; s < S; ++s)
        src << "        _gpu->unif[q][u++] = " << info.scalars[s] << ";\n";
    src << "        _gpu->unif[q][u++] = q;\n"
        << "        _gpu->unif_ptr[q] = _gpu->mail[1]"
        << " + q * " << NAME << "_NUM_UNIFS * sizeof(uint32_t);\n"
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

    write_file(dir + filename + "-qpu.h", hdr.str());
    std::cout << "wrote " << dir + filename + "-qpu.h" << "\n";
    write_file(dir + filename + "-qpu.c", src.str());
    std::cout << "wrote " << dir + filename + "-qpu.c" << "\n";
}

} // namespace qpudsl
