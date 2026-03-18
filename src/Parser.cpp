#include "Parser.h"
#include "Frontend.h"
#include "Type.h"

#include <cctype>
#include <fstream>
#include <map>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace qpudsl {


enum class TK {
    IDENT, INT, STRING,
    PLUS, MINUS, STAR,
    EQ, LPAREN, RPAREN, LBRACKET, RBRACKET, COLON, COMMA,
    END
};

struct Token {
    TK          kind;
    std::string text;
    int         line;
};

static std::vector<Token> lex(const std::string &src) {
    std::vector<Token> tokens;
    int i = 0;
    int n = (int)src.size();
    int line = 1;

    auto error = [&](const std::string &msg) {
        throw std::runtime_error("line " + std::to_string(line) + ": " + msg);
    };

    while (i < n) {
        
        if (src[i] == '\n') { ++line; ++i; continue; }
        if (std::isspace((unsigned char)src[i])) { ++i; continue; }

        // Skip line comments
        if (i + 1 < n && src[i] == '/' && src[i+1] == '/') {
            while (i < n && src[i] != '\n') ++i;
            continue;
        }

        // String literal
        if (src[i] == '"') {
            ++i;
            std::string s;
            while (i < n && src[i] != '"') {
                if (src[i] == '\n') error("unterminated string");
                s += src[i++];
            }
            if (i >= n) error("unterminated string");
            ++i; 
            tokens.push_back({TK::STRING, s, line});
            continue;
        }

        // Integer 
        if (std::isdigit((unsigned char)src[i])) {
            std::string s;
            while (i < n && std::isdigit((unsigned char)src[i])) s += src[i++];
            tokens.push_back({TK::INT, s, line});
            continue;
        }

        // Identifier
        if (std::isalpha((unsigned char)src[i]) || src[i] == '_') {
            std::string s;
            while (i < n && (std::isalnum((unsigned char)src[i]) || src[i] == '_'))
                s += src[i++];
            tokens.push_back({TK::IDENT, s, line});
            continue;
        }

        // Single-character tokens
        TK k;
        switch (src[i]) {
            case '+': k = TK::PLUS;     break;
            case '-': k = TK::MINUS;    break;
            case '*': k = TK::STAR;     break;
            case '=': k = TK::EQ;       break;
            case '(': k = TK::LPAREN;   break;
            case ')': k = TK::RPAREN;   break;
            case '[': k = TK::LBRACKET; break;
            case ']': k = TK::RBRACKET; break;
            case ':': k = TK::COLON;    break;
            case ',': k = TK::COMMA;    break;
            default:
                error("unexpected character '" + std::string(1, src[i]) + "'");
                k = TK::END; 
        }
        tokens.push_back({k, std::string(1, src[i]), line});
        ++i;
    }

    tokens.push_back({TK::END, "", line});
    return tokens;
}


struct Parser {
    const std::vector<Token> &toks;
    int pos = 0;

    std::vector<ParsedTensorDecl>             tensors;
    std::vector<ParsedScalarDecl>             scalars;
    std::vector<ParsedSliceRef>               slice_refs;
    std::map<std::string, int> tensor_map; 
    std::map<std::string, int> scalar_map; 
    std::map<std::string, int>                ref_count; 


    const Token &peek() const { return toks[pos]; }
    const Token &consume() { return toks[pos++]; }

    bool at(TK k) const { return toks[pos].kind == k; }

    const Token &expect(TK k, const char *desc) {
        if (!at(k))
            throw std::runtime_error(
                "line " + std::to_string(peek().line) +
                ": expected " + desc + ", got '" + peek().text + "'");
        return consume();
    }

    bool try_consume(TK k) {
        if (at(k)) { ++pos; return true; }
        return false;
    }

    
    // signed_int 
    int64_t parse_signed_int() {
        bool neg = try_consume(TK::MINUS);
        const Token &t = expect(TK::INT, "integer");
        int64_t v = std::stoll(t.text);
        return neg ? -v : v;
    }

    // Parse dtype string
    dType string_to_dtype(const std::string &s, int line) {
        if (s == "float" || s == "FLOAT32" || s == "f32") return dType::FLOAT32;
        if (s == "int"   || s == "INT32"   || s == "i32") return dType::INT32;
        throw std::runtime_error("line " + std::to_string(line) +
                                 ": unknown dtype '" + s + "'");
    }

    // Parse makeTensor(d0, d1, ..., [dtype])
    void parse_make_tensor(const std::string &name, int line) {
        expect(TK::LPAREN, "(");
        ParsedTensorDecl decl;
        decl.name  = name;
        decl.dtype = dType::INT32;
        while (!at(TK::RPAREN) && !at(TK::END)) {
            if (at(TK::STRING)) {
                decl.dtype = string_to_dtype(consume().text, line);
            } else {
                decl.dims.push_back(parse_signed_int());
            }
            if (!try_consume(TK::COMMA)) break;
        }
        expect(TK::RPAREN, ")");
        if (decl.dims.empty())
            throw std::runtime_error("line " + std::to_string(line) +
                                     ": makeTensor requires at least one dimension");
        tensor_map[name] = (int)tensors.size();
        tensors.push_back(std::move(decl));
    }

    // Parse makeScalar(value, [dtype])
    void parse_make_scalar(const std::string &name, int line) {
        expect(TK::LPAREN, "(");
        ParsedScalarDecl decl;
        decl.name  = name;
        decl.dtype = dType::INT32;
        decl.value = parse_signed_int();
        if (try_consume(TK::COMMA)) {
            if (at(TK::STRING))
                decl.dtype = string_to_dtype(consume().text, line);
        }
        expect(TK::RPAREN, ")");
        scalar_map[name] = (int)scalars.size();
        scalars.push_back(std::move(decl));
    }

    // Parse a declaration: Identifier = Identifier
    void parse_declaration() {
        const Token &lhs = expect(TK::IDENT, "identifier");
        expect(TK::EQ, "=");
        const Token &fn  = expect(TK::IDENT, "function name");
        int ln = fn.line;
        if (fn.text == "makeTensor")
            parse_make_tensor(lhs.text, ln);
        else if (fn.text == "makeScalar")
            parse_make_scalar(lhs.text, ln);
        else
            throw std::runtime_error("line " + std::to_string(ln) +
                                     ": unknown constructor '" + fn.text + "'");
    }

    // slicedim 
    std::pair<int64_t,int64_t> parse_slicedim() {
        int64_t start = parse_signed_int();
        expect(TK::COLON, ":");
        int64_t end   = parse_signed_int();
        return {start, end};
    }

    // factor := Iddentifier[slicelist] or (expression)
    Expr parse_factor() {
        if (at(TK::LPAREN)) {
            consume();
            Expr e = parse_expression();
            expect(TK::RPAREN, ")");
            return e;
        }

        const Token &t = expect(TK::IDENT, "identifier");
        const std::string &name = t.text;

        if (tensor_map.count(name)) {
            ParsedTensorDecl &decl = tensors[tensor_map[name]];
            int N = (int)decl.dims.size();

            // Parse slice list
            std::vector<std::pair<int64_t,int64_t>> slices;
            if (try_consume(TK::LBRACKET)) {
                slices.push_back(parse_slicedim());
                while (try_consume(TK::COMMA))
                    slices.push_back(parse_slicedim());
                expect(TK::RBRACKET, "]");
            }
            while ((int)slices.size() < N)
                slices.push_back({0, -1});

            int idx = ref_count[name]++;
            std::string gen = name + "_" + std::to_string(idx);

            std::vector<Level> levels;
            levels.reserve(N);
            for (int d = 0; d < N; ++d)
                levels.push_back(Level{"d" + std::to_string(d)});
            TensorType tt(Format::ordered(std::move(levels)), decl.dtype);

            slice_refs.push_back({name, gen, slices});
            return Tensor::make(tt, gen);
        }

        if (scalar_map.count(name)) {
            ParsedScalarDecl &decl = scalars[scalar_map[name]];
            return Scalar::make(name, decl.dtype);
        }

        throw std::runtime_error("line " + std::to_string(t.line) +
                                 ": undefined identifier '" + name + "'");
    }

    // term := factor (* factor)*
    Expr parse_term() {
        Expr e = parse_factor();
        while (at(TK::STAR)) {
            consume();
            e = Mul::make(e, parse_factor());
        }
        return e;
    }

    // expression := term ((+/-) term)*
    Expr parse_expression() {
        Expr e = parse_term();
        while (at(TK::PLUS) || at(TK::MINUS)) {
            bool is_sub = at(TK::MINUS);
            consume();
            Expr rhs = parse_term();
            e = is_sub ? Sub::make(e, rhs) : Add::make(e, rhs);
        }
        return e;
    }

    ParsedProgram run() {
        Expr expr;
        bool has_expr = false;

        while (!at(TK::END)) {
            if (at(TK::IDENT) && pos + 1 < (int)toks.size() && toks[pos+1].kind == TK::EQ) {
                parse_declaration();
            } else {
                if (has_expr)
                    throw std::runtime_error(
                        "line " + std::to_string(peek().line) +
                        ": only one expression is supported");
                expr = parse_expression();
                has_expr = true;
            }
        }

        if (!has_expr)
            throw std::runtime_error("DSL has no expression");

        ParsedProgram prog;
        prog.tensors    = std::move(tensors);
        prog.scalars    = std::move(scalars);
        prog.slice_refs = std::move(slice_refs);
        prog.expr       = std::move(expr);
        return prog;
    }
};

ParsedProgram parse_dsl(const std::string &src) {
    auto tokens = lex(src);
    Parser p{tokens};
    return p.run();
}

ParsedProgram parse_dsl_file(const std::string &path) {
    std::ifstream f(path);
    if (!f)
        throw std::runtime_error("cannot open file: " + path);
    std::ostringstream ss;
    ss << f.rdbuf();
    return parse_dsl(ss.str());
}

} // namespace qpudsl
