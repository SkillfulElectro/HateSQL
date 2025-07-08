#include <iostream>
#include "headers/HateSQL_stack.h"

struct RandomObj
{
    int num;
    const char *var;
};

int main()
{
    HateSQL::Stack<int> vec;
    vec.open("test.db");

    vec.push_back(1);
    vec.push_back(2);
    vec.push_back(3);
    vec.push_back(4);

    std::cout << vec.size() << "\n";

    vec.close();
    return 0;
}