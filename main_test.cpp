#include <iostream>
#include <string>
#include "headers/HateSQL_vector.h"
#include "headers/HateSQL_hashmap.h"

int main()
{
    // --- VECTOR TEST ---
    const std::string vec_file = "vec_test.db";
    HateSQL::Vector<int> vec;

    if (HateSQL::Vector<int>::exists(vec_file) == HateSQL::HATESQL_VECTOR_EXISTS) {
        vec.open(vec_file);
    }
    else {
        vec.open(vec_file);
        for (int i = 0; i < 5; ++i) {
            vec.push_back(i);
        }
    }

    // insert 99 at index 2
    vec.insert(2, 99);

    std::cout << "Vector contents: ";
    for (size_t i = 0; i < vec.size(); ++i) {
        std::cout << vec.at_const(i) << (i + 1 < vec.size() ? ", " : "\n");
    }
    vec.close();

    HateSQL::HashMap<const char* , int> map;
    map.open("hashmap.db");

    map.insert("shahin" , 10);
    map.insert("mani" , 2);

    int shahin;
    int mani;

    map.get("shahin" , shahin);
    map.get("mani" , mani);

    std::cout << shahin << " , " << mani << "\n";

    return 0;
}
