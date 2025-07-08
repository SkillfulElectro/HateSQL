#include <iostream>
#include "headers/HateSQL_vector.h"
#include <vector>

struct RandomObj
{
    int num;
    const char *var;
};

int main()
{
    HateSQL::Vector<int> vec;
    

    if (vec.exists("test.db") == HateSQL::HATESQL_VECTOR_EXISTS) {
        vec.open("test.db");

        
    } else {
        vec.open("test.db");

        for (int i = 0 ; i < 5 ; ++i) {
            vec.push_back(i);
        }

        
    }

    vec.insert(vec.size() - 1 , 10);

    for (size_t i = 0 ; i < vec.size() ; ++i) {
        std::cout << vec.at_const(i) << "\n";
    }

    vec.close();

    return 0;
}