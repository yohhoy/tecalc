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


TEST_CASE("number parsing") {
    tecalc::calculator calc;
    REQUIRE(calc.eval(" 0 ") == 0);
    REQUIRE(calc.eval(" 42 ") == 42);

    REQUIRE(calc.eval(" +0 ") == 0);
    REQUIRE(calc.eval(" -0 ") == 0);
    REQUIRE(calc.eval(" +100 ") == 100);
    REQUIRE(calc.eval(" -100 ") == -100);
}

TEST_CASE("add/sub operator") {
    tecalc::calculator calc;
    REQUIRE(calc.eval(" 1 + 2 ") == 3);
    REQUIRE(calc.eval(" 1 - 2 ") == -1);
    REQUIRE(calc.eval(" -1 + +2 ") == 1);
    REQUIRE(calc.eval(" -1 - +2 ") == -3);

    REQUIRE(calc.eval(" 1 + 2 + 3 + 4 ") == 10);
    REQUIRE(calc.eval(" 10 - 5 - 2 ") == 3);
    REQUIRE(calc.eval(" 1 + 2 - 3 ") == 0);
}

TEST_CASE("mul/div operator") {
    tecalc::calculator calc;
    REQUIRE(calc.eval(" 7 * 3 ") == 21);
    REQUIRE(calc.eval(" 7 / 3 ") == 2);
    REQUIRE(calc.eval(" 7 % 3 ") == 1);

    REQUIRE(calc.eval("  7 / -3 ") == -2);
    REQUIRE(calc.eval(" -7 /  3 ") == -2);
    REQUIRE(calc.eval(" -7 / -3 ") == 2);
    REQUIRE(calc.eval("  7 % -3 ") == 1);
    REQUIRE(calc.eval(" -7 %  3 ") == -1);
    REQUIRE(calc.eval(" -7 % -3 ") == -1);

    REQUIRE(calc.eval(" 2 * 3 * 4 ") == 24);
    REQUIRE(calc.eval(" 24 / 2 / 3 ") == 4);
    REQUIRE(calc.eval(" 55 % 10 % 3 ") == 2);
    REQUIRE(calc.eval(" 8 * 6 / 4 % 10 ") == 2);

    REQUIRE(calc.eval(" 1 * 0 ") == 0);
    REQUIRE(calc.eval(" 1 / 0 ") == std::nullopt);
    REQUIRE(calc.eval(" 1 % 0 ") == std::nullopt);
}


TEST_CASE("parenthesis") {
    tecalc::calculator calc;
    REQUIRE(calc.eval(" ( 42 ) ") == 42);
    REQUIRE(calc.eval("((((((((((10))))))))))") == 10);

    REQUIRE(calc.eval("( ") == std::nullopt);
    REQUIRE(calc.eval("(0") == std::nullopt);
    REQUIRE(calc.eval(" )") == std::nullopt);
    REQUIRE(calc.eval("0)") == std::nullopt);
}

TEST_CASE("complex expression") {
    tecalc::calculator calc;
    REQUIRE(calc.eval(" 7 * 3 + 7 / 3 - 7 % 3 ") == 22);
    REQUIRE(calc.eval(" ( 4 - 1 ) * ( -2 + 2 * 5 ) ") == 24);
}
