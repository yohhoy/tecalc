# tecalc - tiny embedded calculator
[![CMake](https://github.com/yohhoy/tecalc/actions/workflows/cmake.yml/badge.svg)](https://github.com/yohhoy/tecalc/actions/workflows/cmake.yml)
[![MIT License](http://img.shields.io/badge/license-MIT-blue.svg)](LICENSE)

This is a tiny expression evaluator, intended for embeding in other C++ program.

```cpp
#include "tecalc.hpp"

auto result = tecalc::eval("(1+2)*3-4");
// result == 5
```

## License
MIT License
