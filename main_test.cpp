#include <iostream>
#include "headers/HateSQL_vector.h"

struct RandomObj
{
    int num;
    const char *var;
};

int main()
{
    HateSQL::Vector<int> vec;
    vec.open("test.db");

    vec.push_back(1);
    vec.push_back(2);
    vec.push_back(3);
    vec.push_back(4);

    vec.erase(0 , 2);

    for (size_t i = 0 ; i < vec.size() ; ++i) {
        std::cout << vec.at(i) << "\n";
    }

    vec.close();
    return 0;
}