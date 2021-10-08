# tecalc - tiny embedded calculator
[![CMake](https://github.com/yohhoy/tecalc/actions/workflows/cmake.yml/badge.svg)](https://github.com/yohhoy/tecalc/actions/workflows/cmake.yml)
[![MIT License](http://img.shields.io/badge/license-MIT-blue.svg)](LICENSE)

This is a tiny expression evaluator, intended for embeding in C++ program.

```cpp
#include "tecalc.hpp"

tecalc::calculator calc;
int answer = calc.eval("(1+2)*3-4");
// answer == 5

calc.set('A', 1);
calc.set('B', 2);
calc.set('C', 3);
answer = calc.eval("(A + B) * C");
// answer == 9
```

## Requirement
- C++17 or later

## License
MIT License
