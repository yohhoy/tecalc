/*
 * tecalc.hpp -- tiny embeded calculator
 *
 * MIT License
 *
 * Copyright 2021 yohhoy
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#ifndef TECALC_HPP_INCLUDED_
#define TECALC_HPP_INCLUDED_

#include <algorithm>
#include <charconv>
#include <map>
#include <optional>
#include <string>
#include <string_view>
#include <system_error>
#include <variant>


namespace tecalc {

//
// error code
//
enum class errc {
    syntax_error = 1,
    invalid_literal,
    unknown_identifier,
    arg_num_mismatch,
    divide_by_zero,
};

namespace impl {

inline const char* errc2msg(errc ev) noexcept
{
    switch (ev) {
    case errc::syntax_error: return "Syntax error";
    case errc::invalid_literal: return "Invalid literal";
    case errc::unknown_identifier: return "Unknown identifier";
    case errc::arg_num_mismatch: return "Argument number mismatch";
    case errc::divide_by_zero: return "Divide by zero";
    }
    return "Unknown tecalc::errc";
}

class tecalc_error_category : public std::error_category {
public:
    const char* name() const noexcept override
        { return "tecalc"; }
    std::string message(int ev) const override
        { return errc2msg(static_cast<errc>(ev)); }
};

//
// meta functions
//
template<class...> struct typelist {};

// repeat<T, N> := typelist<T_n...>
template<class, class> struct repeat_helper {};
template<class T, class... Ts>
struct repeat_helper<T, typelist<Ts...>> { using type = typelist<Ts..., T>; };
template<class T, int N>
struct repeat {
    using type = typename repeat_helper<T, typename repeat<T, N-1>::type>::type;
};
template<class T>
struct repeat<T, 0> { using type = typelist<>; };

// funcptr<T, N> := T(*)(T_1, ...T_n)
template<class R, class... Ts>
auto funcptr_helper(typelist<Ts...>) -> R(*)(Ts...);
template<class T, int N>
struct funcptr {
    using args_typelist = typename repeat<T, N>::type;
    using type = decltype(funcptr_helper<T>(std::declval<args_typelist>()));
};

// func_variant<T, N> := std::variant<T(*)(), T(*)(T_1), ...T(*)(T_1, ...T_n)>
template<class, class> struct func_variant_helper {};
template<class T, class... Ts>
struct func_variant_helper<T, std::variant<Ts...>> { using type = std::variant<Ts..., T>; };
template<class T, int N>
struct func_variant {
    using type = typename func_variant_helper<
        typename funcptr<T, N>::type, typename func_variant<T, N-1>::type>::type;
};
template<class T>
struct func_variant<T, 0> { using type = std::variant<typename funcptr<T, 0>::type>; };

// invoker<Value, F, MaxArgNum>::invoke(f, arg) := std::get<M>(f)(arg[0], ...T_m)
template <class Value, class F, size_t... Is>
inline Value invoke_f(F f, const std::vector<Value>& args, std::index_sequence<Is...>)
{
    return f(args[Is]...);
}
template <class Value, class FnType, size_t N>
struct invoker {
    static inline Value invoke(FnType& fn, const std::vector<Value>& args)
    {
        if (args.size() == N) {
            return invoke_f(std::get<N>(fn), args, std::make_index_sequence<N>{});
        }
        return invoker<Value, FnType, N-1>::invoke(fn, args);
    }
};
template <class Value, class FnType>
struct invoker<Value, FnType, 0> {
    static inline Value invoke(FnType& fn, const std::vector<Value>& args)
    {
        if (args.size() == 0) {
            return (std::get<0>(fn))();
        }
        return {}; // not reachable
    }
};

} // namespace impl

//
// error category
//
inline const std::error_category& tecalc_category() noexcept
{
    static impl::tecalc_error_category s_category;
    return s_category;
}

//
// error class
//
class tecalc_error : public std::runtime_error {
    std::error_code ec_;
public:
    tecalc_error(errc ev)
        : std::runtime_error{impl::errc2msg(ev)}
        , ec_{static_cast<int>(ev), tecalc::tecalc_category()} {}
    const std::error_code& code() const noexcept { return ec_; }
};

} // namespace tecalc


namespace std {
// We define overloads for tecalc::errc in std namespace.
template <>
struct is_error_code_enum<tecalc::errc> : public true_type {};

error_code make_error_code(tecalc::errc e) noexcept
{
    return error_code(static_cast<int>(e), tecalc::tecalc_category());
}

} // namespace std


namespace tecalc {

//
// calculator class-templte
//
template <class Value, int MaxArgNum = 2>
class basic_calculator {
public:
    using value_type = Value;
    using vartbl_type = std::map<std::string, value_type, std::less<>>;

    // function support
    static constexpr int kMaxArgNum = MaxArgNum;
    using func_type = typename impl::func_variant<value_type, kMaxArgNum>::type;
    using functbl_type = std::map<std::string, func_type, std::less<>>;

    // evaluate expression string, return optional<Value> or error_code
    std::optional<value_type> eval(std::string_view expr, std::error_code& ec)
    {
        ptr_ = expr.data();
        last_ = expr.data() + expr.length();
        last_id_ = {};
        last_errc_ = errc{};
        auto res = eval_addsub();
        if (eat_ws()) {
            // We treat as syntax error when unevaluated redundant subsequent characters remain.
            res = std::nullopt;
        }
        if (!res) {
            // If last_errc_ is unset and result is nullopt, report as generic syntax error.
            ec = std::make_error_code(last_errc_ != errc{} ? last_errc_ : errc::syntax_error);
        }
        return res;
    }

    // evaluate expression string, return Value or throw tecalc_error
    value_type eval(std::string_view expr)
    {
        std::error_code ec;
        auto res = eval(expr, ec);
        if (ec.value()) {
            throw tecalc_error(static_cast<errc>(ec.value()));
        }
        return *res;
    }

    // bind value to variable name
    basic_calculator& bind_var(std::string name, value_type val)
    {
        functbl_.erase(name);
        vartbl_[name] = val;
        return *this;
    }

    // bind function pointer to function name
    basic_calculator& bind_fn(std::string name, func_type fn)
    {
        vartbl_.erase(name);
        functbl_[name] = std::move(fn);
        return *this;
    }

private:
    // input expression [ptr_, last_)
    const char* ptr_;
    const char* last_;
    // variable (name, value) pair table
    vartbl_type vartbl_;
    // function (name, funcptr) pair table
    functbl_type functbl_;
    // last parsed identifier (variable or function)
    std::string_view last_id_;
    // last error code
    errc last_errc_;

private:
    // skip consecutive whitespace characters
    bool eat_ws() noexcept
    {
        while (ptr_ != last_ && (*ptr_ == ' ' || *ptr_ == '\t')) {
            ++ptr_;
        }
        return ptr_ != last_;
    }

    // consume string if it exists
    bool consume_str(const char* s) noexcept
    {
        const char* p = ptr_;
        for (; *s; ++s, ++p) {
            if (p == last_ || *p != *s)
                return false;
        }
        ptr_ = p;
        return true;
    }

    // consume one character if it exists
    bool consume_ch(char ch) noexcept
    {
        if (ptr_ == last_ || *ptr_ != ch)
            return false;
        ++ptr_;
        return true;
    }

    // consume one character if it exists, and return it
    char consume_any(std::initializer_list<char> chs)
    {
        if (ptr_ != last_ && std::find(chs.begin(), chs.end(), *ptr_) != chs.end()) {
            return *ptr_++;
        }
        return {};
    }

    // C++ Standard guarantees that digit character codes ('0'-'9') are contiguous,
    // whereas it does not alphabet character codes ('a'-'z', 'A'-'Z') are.
    // Here, we assume ANSI-compatible character set that codes of alphabet are contiguous.
    static bool isdigit(char x) noexcept { return ('0' <= x && x <= '9'); }
    static bool isalpha(char x) noexcept { return ('a' <= x && x <= 'z') || ('A' <= x && x <= 'Z'); }
    static bool isalnum(char x) noexcept { return isdigit(x) || isalpha(x); }

    // integer := {0-9}+
    //         | {"0x"|"0X"} {0-9|a-f|A-F}+
    //         | {"0b"|"0B"} {'0'|'1'}+
    std::optional<value_type> parse_int()
    {
        value_type val;
        int base = 10;
        if (consume_str("0x") || consume_str("0X")) {
            base = 16;
        } else if (consume_str("0b") || consume_str("0B")) {
            base = 2;
        }
        auto [p, ec] = std::from_chars(ptr_, last_, val, base);
        if (ec != std::errc{}) {
            last_errc_ = errc::invalid_literal;
            return {};
        }
        if (p != last_ && isalnum(*p)) {
            last_errc_ = errc::invalid_literal;
            return {};
        }
        ptr_ = p;
        return val;
    }

    // identifier := {a-z|A-Z} {a-z|A-Z|0-9}*
    const char* parse_id()
    {
        if (!isalpha(*ptr_)) return {};
        const char* begin = ptr_;
        for (; ptr_ != last_ && isalnum(*ptr_); ptr_++)
            ;
        return begin;   // [begin, ptr_)
    }

    // primary := '(' addsub ')'
    //          | integer
    //          | identifier
    std::optional<value_type> eval_primary()
    {
        if (!eat_ws()) return {};
        if (consume_ch('(')) {
            auto res = eval_addsub();
            if (!eat_ws()) return {};
            if (!consume_ch(')')) return {};
            return res;
        } else if (isdigit(*ptr_)) {
            return parse_int();
        } else {
            auto name = parse_id();
            if (!name) return {};
            last_id_ = {name, static_cast<size_t>(ptr_ - name)};
            // Here we try to resolve identifier as variable name.
            // If it isn't variable, handle in caller eval_postfix().
            auto var = vartbl_.find(last_id_);
            if (var == vartbl_.end()) {
                last_errc_ = errc::unknown_identifier;
                return {};
            }
            last_id_ = {}; // resolved as variable name
            return var->second;
        }
    }

    // postfix   := primary {'(' arguments? ')'}?
    // arguments := addsub {',' addsub}*
    std::optional<value_type> eval_postfix()
    {
        auto res = eval_primary();
        if (!res && last_errc_ != errc::unknown_identifier) return {};
        eat_ws();
        if (consume_ch('(')) {
            if (last_id_.empty()) {
                // When non-function name followed by '(', report syntax error.
                last_errc_ = errc::syntax_error;
                return {};
            }
            // resolve as function name
            auto func = functbl_.find(last_id_);
            if (func == functbl_.end()) {
                last_errc_ = errc::unknown_identifier;
                return {};
            }
            last_id_ = {};
            // evaluate arguments list
            std::vector<value_type> args;
            while (eat_ws()) {
                char op = consume_any({',', ')'});
                if (op == ')') break;
                auto arg = eval_addsub();
                if (!arg) return {};
                args.push_back(*arg);
            }
            // invoke user-defined function
            if (func->second.index() != args.size()) {
                last_errc_ = errc::arg_num_mismatch;
                return {};
            }
            using invoker = impl::invoker<value_type, func_type, kMaxArgNum>;
            res = invoker::invoke(func->second, args);
        } else if (!last_id_.empty()) {
            if (functbl_.find(last_id_) != functbl_.end()) {            
                // When function name followed by non-'(', report syntax error.
                last_errc_ = errc::syntax_error;
                return {};
            }
        }
        return res;
    }

    // unary := {'-'|'+'}* primary
    std::optional<value_type> eval_unary()
    {
        bool neg = false;
        char op;
        do {
            if (!eat_ws()) return {};
            op = consume_any({'+', '-'});
            neg ^= (op == '-');
        } while (op);
        auto res = eval_postfix();
        if (res && neg) {
            return -*res;
        } else {
            return res;
        }
    }

    // muldiv := unary {'*'|'/'|'%' unary}*
    std::optional<value_type> eval_muldiv()
    {
        auto lhs = eval_unary();
        if (!lhs) return {};
        value_type res = *lhs;
        while (eat_ws()) {
            char op = consume_any({'*', '/', '%'});
            if (!op) return res;
            auto rhs = eval_unary();
            if (!rhs) return {};
            if (op == '*') {
                res *= *rhs;
            } else {
                if (*rhs == 0) {
                    last_errc_ = errc::divide_by_zero;
                    return {};
                }
                if (op == '/') {
                    res /= *rhs;
                } else {
                    res %= *rhs;
                }
            }
        }
        return res;
    }

    // addsub := muldiv {'+'|'-' muldiv}*
    std::optional<value_type> eval_addsub()
    {
        auto lhs = eval_muldiv();
        if (!lhs) return {};
        value_type res = *lhs;
        while (eat_ws()) {
            char op = consume_any({'+', '-'});
            if (!op) return res;
            auto rhs = eval_muldiv();
            if (!rhs) return {};
            if (op == '+') {
                res += *rhs;
            } else {
                res -= *rhs;
            }
        }
        return res;
    }
};

using calculator = basic_calculator<int>;

} // namespace tecalc

#endif
