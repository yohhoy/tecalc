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
#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>
#include "tecalc.hpp"


TEST_CASE("integer literals") {
    tecalc::calculator calc;
    // decimal literal
    REQUIRE(calc.eval(" 0 ") == 0);
    REQUIRE(calc.eval(" 100 ") == 100);
    REQUIRE(calc.eval("00000000000000000042") == 42);
    // hexadecimal literal
    REQUIRE(calc.eval(" 0x2a ") == 42);
    REQUIRE(calc.eval(" 0X2A ") == 42);
    REQUIRE(calc.eval("0x00000000000000002A") == 42);
    // binary literal
    REQUIRE(calc.eval(" 0b1010 ") == 10);
    REQUIRE(calc.eval(" 0B0101 ") == 5);
    REQUIRE(calc.eval("0b000000000000000010") == 2);
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
    REQUIRE(calc.eval(" 1 * 0 ") == 0);
    REQUIRE(calc.eval(" 1 / 0 ") == std::nullopt);
    REQUIRE(calc.eval(" 1 % 0 ") == std::nullopt);
}

TEST_CASE("parenthesis") {
    tecalc::calculator calc;
    REQUIRE(calc.eval(" ( 42 ) ") == 42);
    REQUIRE(calc.eval("((((((((((10))))))))))") == 10);
    // unmatched parenthesis
    REQUIRE(calc.eval(" (  ") == std::nullopt);
    REQUIRE(calc.eval(" (0 ") == std::nullopt);
    REQUIRE(calc.eval("((0)") == std::nullopt);
    REQUIRE(calc.eval("  ) ") == std::nullopt);
    REQUIRE(calc.eval(" 0) ") == std::nullopt);
    REQUIRE(calc.eval("(0))") == std::nullopt);
}

TEST_CASE("complex expression") {
    tecalc::calculator calc;
    REQUIRE(calc.eval("7 * 3 + 7 / 3 - 7 % 3") == 22);
    REQUIRE(calc.eval("(4 - 1) * (-2 + 2 * 5)") == 24);
    REQUIRE(calc.eval("--1--1--1--1--1") == 5);
    REQUIRE(calc.eval("-+1+-1-+1+-1-+1") == -5);
}

TEST_CASE("variables") {
    tecalc::calculator calc;
    REQUIRE(calc.set("x", 1) == std::nullopt);
    REQUIRE(calc.set("y", 2) == std::nullopt);
    REQUIRE(calc.set("x", 3) == 1);
    // use vars
    REQUIRE(calc.eval(" x ") == 3);
    REQUIRE(calc.eval("(x)") == 3);
    REQUIRE(calc.eval(" x * y ") == 6);
    REQUIRE(calc.eval("+x*-y") == -6);
    calc.set("K1", 10); calc.set("K2", 20); calc.set("K3", 30);
    REQUIRE(calc.eval("K1 * (K2 + K3)") == 500);
}
