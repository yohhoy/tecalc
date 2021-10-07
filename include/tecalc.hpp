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

#include <optional>
#include <charconv>

namespace tecalc {

namespace impl {

inline bool eat_ws(const char*& s, const char* last)
{
    while (s != last && (*s == ' ' || *s == '\t')) {
        ++s;
    }
    return s != last;
}

} // namespace impl


template <class Value>
class basic_calculator {
public:
    using value_type = Value;

    std::optional<value_type>
    eval(const char* expr)
    {
        auto last = expr + std::char_traits<char>::length(expr);
        auto res = eval_addsub(expr, last);
        if (impl::eat_ws(expr, last)) return {};
        return res;
    }

private:
    // integer = ['+'|'-']? ['0'..'9']+
    std::optional<value_type> parse_int(const char*& expr, const char* last)
    {
        if (expr != last && *expr == '+') {
            // ignore '+' because std::from_chars doesn't parse it.
            ++expr;
        }
        value_type val;
        auto [p, ec] = std::from_chars(expr, last, val);
        if (ec == std::errc{}) {
            expr = p;
            return val;
        }
        return {};
    }

    // pexpr := '(' addsub ')'
    // pexpr := integer
    std::optional<value_type> eval_pexpr(const char*& expr, const char* last)
    {
        if (!impl::eat_ws(expr, last)) return {};
        if (*expr == '(') {
            ++expr;
            auto res = eval_addsub(expr, last);
            if (!impl::eat_ws(expr, last)) return {};
            if (*expr != ')') return {};
            ++expr;
            return res;
        } else {
            return parse_int(expr, last);
        }
    }

    // muldiv := pexpr ['*'|'/'|'%' pexpr]*
    std::optional<value_type> eval_muldiv(const char*& expr, const char* last)
    {
        auto lhs = eval_pexpr(expr, last);
        value_type res = *lhs;
        if (!lhs) return {};
        while (impl::eat_ws(expr, last)) {
            char op = *expr;
            if (op != '*' && op != '/' && op != '%') {
                return res;
            }
            ++expr;
            auto rhs = eval_pexpr(expr, last);
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
    std::optional<value_type> eval_addsub(const char*& expr, const char* last)
    {
        auto lhs = eval_muldiv(expr, last);
        if (!lhs) return {};
        value_type res = *lhs;
        while (impl::eat_ws(expr, last)) {
            char op = *expr;
            if (op != '+' && op != '-') {
                return res;
            }
            ++expr;
            auto rhs = eval_muldiv(expr, last);
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
