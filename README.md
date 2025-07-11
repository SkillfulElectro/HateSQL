# HateSQL: A No-Query Database

HateSQL is a minimalist, header-only C++ library that makes file-based data storage as simple and intuitive as working with `std::vector` and plain old data (POD) types.

## Goals
- Ease of use: interact with your data as naturally as you would with `std::vector`.
- Zero dependencies: no TCP connections, no external libraries—just include the headers and you’re ready to go.

## Limitations
- Only POD types, fixed-length arrays, and aggregates composed of them are supported.
- No support for dynamic types, complex types .

check main_test.cpp as an usage example .
