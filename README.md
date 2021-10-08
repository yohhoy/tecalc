# tecalc - tiny embedded calculator
[![CMake](https://github.com/yohhoy/tecalc/actions/workflows/cmake.yml/badge.svg)](https://github.com/yohhoy/tecalc/actions/workflows/cmake.yml)
[![MIT License](http://img.shields.io/badge/license-MIT-blue.svg)](LICENSE)

This is a tiny numerical expression evaluator, intended for embeding in C++ program.

```cpp
#include "tecalc.hpp"

tecalc::calculator calc;
calc.set("A", 2);
calc.set("B", 4);
int answer = calc.eval("(1 + A) * B - 2");
// answer == 10
```

## Requirement
- C++17 or later

## Grammer
```
expression   := addsub-expr
addsub-expr  := muldiv-expr {'+'|'-' muldiv-expr}*
muldiv-expr  := unary-expr {'*'|'/'|'%' unary-expr}*
unary-expr   := {'+'|'-'}* primary-expr
primary-expr := {'(' expression ')'} | integer | variable

integer  := {digit}+  // decimal
          | {"0x"|"0X"} {digit | 'a'|...|'f' | 'A'|...|'F'}+  // hexadecimal
          | {"0b"|"0B"} {'0'|'1'}+  // binary
variable := alphabet {alphabet | digit}*
digit    := '0'|...|'9'
alphabet := 'a'|...|'z' | 'A'|...|'Z'
```

## License
MIT License
