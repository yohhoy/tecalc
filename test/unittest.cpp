/*
 * unittest.cpp
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
#include <type_traits>
#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>
#include "tecalc.hpp"

// int-casted tecalc::errc enumerator for std::error_code::value()
constexpr int syntax_error = static_cast<int>(tecalc::errc::syntax_error);
constexpr int invalid_literal = static_cast<int>(tecalc::errc::invalid_literal);
constexpr int undefined_var = static_cast<int>(tecalc::errc::undefined_var);
constexpr int divide_by_zero = static_cast<int>(tecalc::errc::divide_by_zero);

// custom matcher for tecalc::tecalc_error
auto IsErrc(tecalc::errc ev) {
    struct TecalcErrorMatcher : public Catch::MatcherBase<tecalc::tecalc_error> {
        std::error_code ec_;
        TecalcErrorMatcher(tecalc::errc ev)
            : ec_{std::make_error_code(ev)} {}
        bool match(tecalc::tecalc_error const& e) const override {
            return e.code() == ec_;
        }
        std::string describe() const override {
            std::ostringstream ss;
            ss << "is " << ec_.message();
            return ss.str();
        }
    };
    return TecalcErrorMatcher{ev};
}


TEST_CASE("integer literals") {
    tecalc::calculator calc;
    std::error_code ec;
    // decimal literal
    REQUIRE(calc.eval(" 0 ") == 0);
    REQUIRE(calc.eval(" 100 ") == 100);
    REQUIRE(calc.eval("00000000000000000042") == 42);
    REQUIRE(calc.eval("0a", ec) == std::nullopt); CHECK(ec.value() == invalid_literal);
    REQUIRE(calc.eval("0A", ec) == std::nullopt); CHECK(ec.value() == invalid_literal);
    // hexadecimal literal
    REQUIRE(calc.eval(" 0x2a ") == 42);
    REQUIRE(calc.eval(" 0X2A ") == 42);
    REQUIRE(calc.eval("0x00000000000000002A") == 42);
    REQUIRE(calc.eval("0xG", ec) == std::nullopt); CHECK(ec.value() == invalid_literal);
    REQUIRE(calc.eval("0x8FG", ec) == std::nullopt); CHECK(ec.value() == invalid_literal);
    REQUIRE(calc.eval("0x+0", ec) == std::nullopt); CHECK(ec.value() == invalid_literal);
    // binary literal
    REQUIRE(calc.eval(" 0b1010 ") == 10);
    REQUIRE(calc.eval(" 0B0101 ") == 5);
    REQUIRE(calc.eval("0b000000000000000010") == 2);
    REQUIRE(calc.eval("0b2", ec) == std::nullopt); CHECK(ec.value() == invalid_literal);
    REQUIRE(calc.eval("0b012", ec) == std::nullopt); CHECK(ec.value() == invalid_literal);
    REQUIRE(calc.eval("0b+0", ec) == std::nullopt); CHECK(ec.value() == invalid_literal);
}

TEST_CASE("unary operator") {
    tecalc::calculator calc;
    REQUIRE(calc.eval(" + 0 ") == 0);
    REQUIRE(calc.eval(" - 0 ") == 0);
    REQUIRE(calc.eval(" + 100 ") == 100);
    REQUIRE(calc.eval(" - 100 ") == -100);
    // sequence
    REQUIRE(calc.eval(" + - - - + 42 ") == -42);
}

TEST_CASE("add/sub operator") {
    tecalc::calculator calc;
    REQUIRE(calc.eval(" 1 + 2 ") == 3);
    REQUIRE(calc.eval(" 1 - 2 ") == -1);
    REQUIRE(calc.eval(" -1 + +2 ") == 1);
    REQUIRE(calc.eval(" -1 - +2 ") == -3);
    // sequence
    REQUIRE(calc.eval(" 1 + 2 + 3 + 4 ") == 10);
    REQUIRE(calc.eval(" 10 - 5 - 2 ") == 3);
    REQUIRE(calc.eval(" 1 + 2 - 3 ") == 0);
}

TEST_CASE("mul/div/mod operator") {
    tecalc::calculator calc;
    REQUIRE(calc.eval(" 7 * 3 ") == 21);
    REQUIRE(calc.eval(" 7 / 3 ") == 2);
    REQUIRE(calc.eval(" 7 % 3 ") == 1);
    // divide/modulo with minus
    REQUIRE(calc.eval("  7 / -3 ") == -2);
    REQUIRE(calc.eval(" -7 /  3 ") == -2);
    REQUIRE(calc.eval(" -7 / -3 ") == 2);
    REQUIRE(calc.eval("  7 % -3 ") == 1);
    REQUIRE(calc.eval(" -7 %  3 ") == -1);
    REQUIRE(calc.eval(" -7 % -3 ") == -1);
    // sequence
    REQUIRE(calc.eval(" 2 * 3 * 4 ") == 24);
    REQUIRE(calc.eval(" 24 / 2 / 3 ") == 4);
    REQUIRE(calc.eval(" 55 % 10 % 3 ") == 2);
    REQUIRE(calc.eval(" 8 * 6 / 4 % 10 ") == 2);
    // divide by zero
    std::error_code ec;
    REQUIRE(calc.eval(" 1 * 0 ") == 0);
    REQUIRE(calc.eval(" 1 / 0 ", ec) == std::nullopt); CHECK(ec.value() == divide_by_zero);
    REQUIRE(calc.eval(" 1 % 0 ", ec) == std::nullopt); CHECK(ec.value() == divide_by_zero);
}

TEST_CASE("parenthesis") {
    tecalc::calculator calc;
    REQUIRE(calc.eval(" ( 42 ) ") == 42);
    REQUIRE(calc.eval("((((((((((10))))))))))") == 10);
    // unmatched parenthesis
    std::error_code ec;
    REQUIRE(calc.eval(" (  ", ec) == std::nullopt); CHECK(ec.value() == syntax_error);
    REQUIRE(calc.eval(" (0 ", ec) == std::nullopt); CHECK(ec.value() == syntax_error);
    REQUIRE(calc.eval("((0)", ec) == std::nullopt); CHECK(ec.value() == syntax_error);
    REQUIRE(calc.eval("  ) ", ec) == std::nullopt); CHECK(ec.value() == syntax_error);
    REQUIRE(calc.eval(" 0) ", ec) == std::nullopt); CHECK(ec.value() == syntax_error);
    REQUIRE(calc.eval("(0))", ec) == std::nullopt); CHECK(ec.value() == syntax_error);
    // empty parenthesis
    REQUIRE(calc.eval("()", ec) == std::nullopt); CHECK(ec.value() == syntax_error);
}

TEST_CASE("complex expression") {
    tecalc::calculator calc;
    REQUIRE(calc.eval("7 * 3 + 7 / 3 - 7 % 3") == 22);
    REQUIRE(calc.eval("(4 - 1) * (-2 + 2 * 5)") == 24);
    REQUIRE(calc.eval("--1--1--1--1--1") == 5);
    REQUIRE(calc.eval("-+1+-1-+1+-1-+1") == -5);
    // no expression
    std::error_code ec;
    REQUIRE(calc.eval("", ec) == std::nullopt); CHECK(ec.value() == syntax_error);
    REQUIRE(calc.eval(" ", ec) == std::nullopt); CHECK(ec.value() == syntax_error);
    // redundant subsequent chars
    REQUIRE(calc.eval("1 2", ec) == std::nullopt); CHECK(ec.value() == syntax_error);
}

TEST_CASE("variables") {
    tecalc::calculator calc;
    REQUIRE(calc.set("x", 1) == std::nullopt);
    REQUIRE(calc.set("y", 2) == std::nullopt);
    REQUIRE(calc.set("x", 3) == 1);
    // use variables
    REQUIRE(calc.eval(" x ") == 3);
    REQUIRE(calc.eval("(x)") == 3);
    REQUIRE(calc.eval(" x * y ") == 6);
    REQUIRE(calc.eval("+x*-y") == -6);
    calc.set("K1", 10); calc.set("K2", 20); calc.set("K3", 30);
    REQUIRE(calc.eval("K1 * (K2 + K3)") == 500);
    // undefined varriable
    std::error_code ec;
    REQUIRE(calc.eval("undefined", ec) == std::nullopt); CHECK(ec.value() == undefined_var);
}

TEST_CASE("exception handling") {
    using Catch::Matchers::Equals;
    // error category/error code
    const auto& tecalc_category = tecalc::tecalc_category();
    REQUIRE(tecalc_category == tecalc::tecalc_category());
    REQUIRE_THAT(tecalc_category.name(), Equals("tecalc"));
    REQUIRE_THAT(tecalc_category.message(syntax_error), Equals("Syntax error"));
    REQUIRE_THAT(tecalc_category.message(invalid_literal), Equals("Invalid literal"));
    REQUIRE_THAT(tecalc_category.message(undefined_var), Equals("Undefined variable"));
    REQUIRE_THAT(tecalc_category.message(divide_by_zero), Equals("Divide by zero"));
    // throw tecalc_error
    static_assert(std::is_base_of<std::runtime_error, tecalc::tecalc_error>::value);
    tecalc::calculator calc;
    REQUIRE_THROWS_MATCHES(calc.eval("42+"), tecalc::tecalc_error, IsErrc(tecalc::errc::syntax_error));
    REQUIRE_THROWS_MATCHES(calc.eval("0b2"), tecalc::tecalc_error, IsErrc(tecalc::errc::invalid_literal));
    REQUIRE_THROWS_MATCHES(calc.eval("und"), tecalc::tecalc_error, IsErrc(tecalc::errc::undefined_var));
    REQUIRE_THROWS_MATCHES(calc.eval("0/0"), tecalc::tecalc_error, IsErrc(tecalc::errc::divide_by_zero));
}
