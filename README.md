# HateSQL: A No-Query Database

HateSQL is a minimalist, header-only C++ library that makes file-based data storage as simple and intuitive as working with `std::vector` and plain old data (POD) types.

## Goals
- Ease of use: interact with your data as naturally as you would with `std::vector`.
- Zero dependencies: no TCP connections, no external libraries—just include the headers and you’re ready to go.

## Limitations
- Only POD types, fixed-length arrays, and aggregates composed of them are supported.
- No support for dynamic types, complex types .

## modules
all of the modules support multiple read but single write operation , so you can bind as many as you want of them to a file .
- `HateSQL::Vector` : simple `std::vector` like class which manages the memory within the file .

- `HateSQL::HashMap` : just simple wrapper on top of `HateSQL::Vector` which tries to implement simple hashmap and uses lazy rehashing algorithm .

- `HateSQL::DynVector` : `HateSQL::Vector` but any type for values can be used .
- `HateSQL::DynHashMap` : `HateSQL::HashMap` but any type can be used as value .



check main_test.cpp as an usage example .
