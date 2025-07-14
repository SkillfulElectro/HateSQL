#include <iostream>
#include <string>
#include "headers/HateSQL.h"

struct Char20 {
    char data[20];
};

namespace std {
    template<>
    struct hash<Char20> {
        size_t operator()(Char20 const& c) const noexcept {
            return std::hash<char[20]>()(c.data);
        }
    };
}


int main()
{
    // --- VECTOR TEST ---
    const std::string vec_file = "str_vec_test.db";
    HateSQL::Vector<int> vec;



    if (HateSQL::exists(vec_file) == HateSQL::HATESQL_VECTOR_EXISTS) {
        vec.open(vec_file);
    } else {
        vec.open(vec_file);

        for (int i = 0 ; i < 50 ; ++i) {
            vec.push_back(i);
        }




    }

/**/
    int* sample_buffer = new int[10];

    for (int i {0} ; i < 10 ; ++i) {
        sample_buffer[i] = 100;
    }
    vec.buffered_insert(vec.size() - 1 , sample_buffer , 10 , 20);

    delete[] sample_buffer;

    for (size_t i = 0 ; i < vec.size() ; ++i) {
        int tmp;
        vec.get(i , tmp);
        std::cout << i << " " << tmp << "\n";
    }

    vec.close();
    
    HateSQL::HashMap<Char20 , int> map;



    if (HateSQL::exists("hashmap.db") == HateSQL::HATESQL_VECTOR_EXISTS) {
        map.open("hashmap.db");
    } else {
        map.open("hashmap.db");
        map.insert({"shahin"} , 10);
        map.insert({"mani"} , 50);
        map.insert({"ghasem"} , 80);
    }
    


    int shahin;
    int mani;


    std::cout << "shahin found : " << map.get({"shahin"} , shahin) << "\n";
    std::cout << "mani found : " << map.get({"mani"} , mani) << "\n";

    std::cout << shahin << " , " << mani << "\n";

    map.close();

    return 0;
}