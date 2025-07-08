#include <iostream>
#include "headers/HateSQL_stack.h"

struct RandomObj {
    int num;
    const char * var;
};

int main() {
    HateSQL::Stack<int> vec;
    vec.open("test.db");



    std::cout << vec.size() << "\n";

    vec.close();
    return 0;
}
