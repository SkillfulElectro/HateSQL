#include <iostream>
#include <string>
#include "headers/HateSQL_vector.h"
#include "headers/HateSQL_hashmap.h"


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

    HateSQL::HashMap<char[20] , int> map;

    char shah[20] = "shahin";
    char man[20] = "mani";

    if (HateSQL::exists("hashmap.db") == HateSQL::HATESQL_VECTOR_EXISTS) {
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