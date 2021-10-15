# tecalc - tiny embedded calculator
[![CMake](https://github.com/yohhoy/tecalc/actions/workflows/cmake.yml/badge.svg)](https://github.com/yohhoy/tecalc/actions/workflows/cmake.yml)
[![MIT License](http://img.shields.io/badge/license-MIT-blue.svg)](LICENSE)

This is a tiny numerical expression evaluator, intended for embeding in C++ program.

```cpp
#include "tecalc.hpp"

tecalc::calculator calc;
calc.bind_var("A", 2).bind_var("B", 4);
int res1 = calc.eval("(1 + A) * B - 2");
// res1 == 10

calc.bind_fn("abs", [](int x){ return x < 0 ? -x : x; })
    .bind_fn("min", [](int a, int b){ return a < b ? a : b; });
int res2 = calc.eval("abs(min(-A, -B))");
// res2 = 4
```

## Requirement
- C++17 or later

## Grammer
```
expression   := addsub-expr
addsub-expr  := muldiv-expr {'+'|'-' muldiv-expr}*
muldiv-expr  := unary-expr {'*'|'/'|'%' unary-expr}*
unary-expr   := {'+'|'-'}* postfix-expr
postfix-expr := primary-expr {'(' arguments? ')'}?
arguments    := addsub-expr {',' addsub-expr}*
primary-expr := {'(' addsub-expr ')'} | integer | identifier

integer    := {digit}+  // decimal
            | {"0x"|"0X"} {digit | 'a'|...|'f' | 'A'|...|'F'}+  // hexadecimal
            | {"0b"|"0B"} {'0'|'1'}+  // binary
identifier := alphabet {alphabet | digit}*  // variable or function
digit      := '0'|...|'9'
alphabet   := 'a'|...|'z' | 'A'|...|'Z'
```

## License
MIT License
