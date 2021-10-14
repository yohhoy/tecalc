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


namespace tecalc {

//
// error code
//
enum class errc {
    syntax_error = 1,
    invalid_literal,
    undefined_var,
    divide_by_zero,
};

namespace impl {

inline const char* errc2msg(errc ev) noexcept
{
    switch (ev) {
    case errc::syntax_error: return "Syntax error";
    case errc::invalid_literal: return "Invalid literal";
    case errc::undefined_var: return "Undefined variable";
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
template <class Value>
class basic_calculator {
public:
    using value_type = Value;
    using vartbl_type = std::map<std::string, value_type>;

    // evaluate expression string, return optional<Value> or error_code
    std::optional<value_type> eval(std::string_view expr, std::error_code& ec)
    {
        ptr_ = expr.data();
        last_ = expr.data() + expr.length();
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

    // set value to named variable
    std::optional<value_type> set(std::string name, value_type val)
    {
        auto itr = vartbl_.find(name);
        if (itr != vartbl_.end()) {
            // update value of variable and return old value
            return std::exchange(itr->second, val);
        } else {
            // register value as new variable
            vartbl_.emplace(name, val);
            return {};
        }
    }

private:
    // input expression [ptr_, last_)
    const char* ptr_;
    const char* last_;
    // variable (name, value) pair table
    vartbl_type vartbl_;
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

    // variable := {a-z|A-Z} {a-z|A-Z|0-9}*
    std::optional<value_type> eval_var()
    {
        if (!isalpha(*ptr_)) return {};
        std::string name;
        do {
            name.push_back(*ptr_++);
        } while (ptr_ != last_ && isalnum(*ptr_));
        auto itr = vartbl_.find(name);
        if (itr == vartbl_.end()) {
            last_errc_ = errc::undefined_var;
            return {};
        }
        return itr->second;
    }

    // primary := '(' addsub ')'
    //          | integer
    //          | variable
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
            return eval_var();
        }
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
        auto res = eval_primary();
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
