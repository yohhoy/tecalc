# tecalc - tiny embedded calculator
[![CMake](https://github.com/yohhoy/tecalc/actions/workflows/cmake.yml/badge.svg)](https://github.com/yohhoy/tecalc/actions/workflows/cmake.yml)
[![MIT License](http://img.shields.io/badge/license-MIT-blue.svg)](LICENSE)

This is a tiny numerical expression evaluator, intended for embedding in C++ program.

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

The `tecalc` evaluator supports;
- basic arithmetic operators such as `+`, `-`, `*`, `/`, `%`(modulo) and parentheses,
- decimal/hexadecimal(`0x`)/binary(`0b`) number literals,
- binding a value to variable name, and user-defined function call.

The `eval` uses single-pass algorithm, builds no AST (abstract syntax tree),
parses and evaluates input expressions simultaneously.

## Requirement
- C++17 or later

## API
```c++
namespace tecalc {
// error codes
enum class errc {...};

// exception throw by eval()
class tecalc_error : public std::runtime_error {
    const std::error_code& code();
    // (and inherited from std::runtime_error)
};

template <class Value, int MaxArgNum = 2>
class basic_calculator {
    using value_type = Value;
    using func_type = std::variant</*see below*/>;
    // std::variant of function types that different number of parameters
    // Value(*)(), Value(*)(Value), Value(*)(Value,Value), ...

    // evaluate expression string, return optional<Value> or error_code
    std::optional<value_type> eval(std::string_view expr, std::error_code& ec);
    // evaluate expression string, return Value or throw tecalc_error
    value_type eval(std::string_view expr);
    
    // bind value to variable name
    basic_calculator& bind_var(std::string name, value_type val);
    // bind function pointer to function name
    basic_calculator& bind_fn(std::string name, func_type fn);
};

using calculator = basic_calculator<int>;
}
```

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
