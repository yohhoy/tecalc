# tecalc - tiny embedded calculator
[![CMake](https://github.com/yohhoy/tecalc/actions/workflows/cmake.yml/badge.svg)](https://github.com/yohhoy/tecalc/actions/workflows/cmake.yml)
[![MIT License](http://img.shields.io/badge/license-MIT-blue.svg)](LICENSE)

This is a tiny expression evaluator, intended for embeding in C++ program.

```cpp
#include "tecalc.hpp"

tecalc::calculator calc;
auto result = calc.eval("(1+2)*3-4");
// result == 5
```

## Requirement
- C++17 or later

## License
MIT License
