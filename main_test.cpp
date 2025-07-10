#include <iostream>
#include <string>
#include "headers/HateSQL_vector.h"
#include "headers/HateSQL_hashmap.h"

int main()
{
    // --- VECTOR TEST ---
    const std::string vec_file = "str_vec_test.db";
    HateSQL::Vector<char[20]> vec;

    char hi[20] = "hi";
    char bye[20] = "bye";

    if (HateSQL::Vector<char[20]>::exists(vec_file) == HateSQL::HATESQL_VECTOR_EXISTS) {
        vec.open(vec_file);
    } else {
        vec.open(vec_file);

        vec.push_back(hi);
        vec.push_back(bye);
    }






    for (size_t i = 0 ; i < vec.size() ; ++i) {
        char tmp[20];
        vec.get(i , tmp);
        std::cout << tmp << "\n";
    }

    vec.close();

    HateSQL::HashMap<char[20] , int> map;

    char shah[20] = "shahin";
    char man[20] = "mani";

    if (HateSQL::HashMap<char[20] , int>::exists("hashmap.db") == HateSQL::HATESQL_VECTOR_EXISTS) {
        map.open("hashmap.db");
    } else {
        map.open("hashmap.db");
        map.insert(shah , 10);
        map.insert(man , 50);
    }
    


    int shahin;
    int mani;

    map.set(shah , 20);

    std::cout << "shahin found : " << map.get(shah , shahin) << "\n";
    std::cout << "mani found : " << map.get(man , mani) << "\n";

    std::cout << shahin << " , " << mani << "\n";

    map.close();

    return 0;
}
