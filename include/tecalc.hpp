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
#include <optional>
#include <string_view>


namespace tecalc {

template <class Value>
class basic_calculator {
    // input expression [ptr_, last_)
    const char* ptr_;
    const char* last_;

public:
    using value_type = Value;

    std::optional<value_type> eval(std::string_view expr)
    {
        ptr_ = expr.begin();
        last_ = expr.end();
        auto res = eval_addsub();
        if (eat_ws()) return {};
        return res;
    }

private:
    // skip consecutive whitespace characters
    bool eat_ws()
    {
        while (ptr_ != last_ && (*ptr_ == ' ' || *ptr_ == '\t')) {
            ++ptr_;
        }
        return ptr_ != last_;
    }

    // consume string if it exists
    bool consume_str(const char* s)
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
    bool consume_ch(char ch)
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

    // integer := ['0'..'9']+
    //          | ["0x"|"0X"]? ['0'..'9'|'a'..'f'|'A'..'F']+
    //          | ["0b"|"0B"]? ['0'|'1']+
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
        if (ec == std::errc{}) {
            ptr_ = p;
            return val;
        }
        return {};
    }

    // primary := '(' addsub ')'
    //          | integer
    std::optional<value_type> eval_primary()
    {
        if (!eat_ws()) return {};
        if (consume_ch('(')) {
            auto res = eval_addsub();
            if (!eat_ws()) return {};
            if (!consume_ch(')')) return {};
            return res;
        } else {
            return parse_int();
        }
    }

    // unary = ['-'|'+']* primary
    std::optional<value_type> eval_unary()
    {
        if (!eat_ws()) return {};
        char op = consume_any({'-', '+'});
        if (!op) return eval_primary();
        auto res = eval_unary();
        if (!res) return {};
        if (op == '-') {
            return -*res;
        } else {
            return +*res;
        }
    }

    // muldiv := unary ['*'|'/'|'%' unary]*
    std::optional<value_type> eval_muldiv()
    {
        auto lhs = eval_unary();
        value_type res = *lhs;
        if (!lhs) return {};
        while (eat_ws()) {
            char op = consume_any({'*', '/', '%'});
            if (!op) return res;
            auto rhs = eval_unary();
            if (!rhs) return {};
            if (op == '*') {
                res *= *rhs;
            } else {
                if (*rhs == 0) return {};
                if (op == '/') {
                    res /= *rhs;
                } else {
                    res %= *rhs;
                }
            }
        }
        return res;
    }

    // addsub := muldiv ['+'|'-' muldiv]*
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
