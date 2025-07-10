#include <iostream>
#include <string>
#include "headers/HateSQL_vector.h"
#include "headers/HateSQL_hashmap.h"

struct Char20 {
    char data[20];
};

int main()
{
    // --- VECTOR TEST ---
    const std::string vec_file = "str_vec_test.db";
    HateSQL::Vector<Char20> vec;



    if (HateSQL::Vector<char[20]>::exists(vec_file) == HateSQL::HATESQL_VECTOR_EXISTS) {
        vec.open(vec_file);
    } else {
        vec.open(vec_file);

        vec.push_back({"hi"});
        vec.push_back({"bye"});
    }


    Char20* buffer = new Char20[vec.size()];

    vec.buffered_get(0 , buffer, vec.size());

    for (size_t i {0} ; i < vec.size() ; ++i) {
        std::cout << buffer[i].data << i << " ";
    }
    std::cout << "\n";

    buffer[0] = Char20{"boz"};
    buffer[1] = Char20{"doz"};

    vec.buffered_set(0 , buffer , vec.size());

    for (size_t i = 0 ; i < vec.size() ; ++i) {
        Char20 tmp;
        vec.get(i , tmp);
        std::cout << tmp.data << "\n";
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
