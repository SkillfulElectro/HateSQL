#ifndef HATE_SQL_HASHMAP
#define HATE_SQL_HASHMAP

#include "HateSQL.h"

/*
HateSQL::HashMap is just wrapper on top of HateSQL::Vector
*/

namespace std
{
    template <size_t N>
    struct hash<char[N]>
    {
        size_t operator()(const char (&s)[N]) const noexcept
        {
            return std::hash<std::string_view>{}(std::string_view(s));
        }
    };
}

namespace HateSQL
{
    template <typename Value>
    struct HashMapData
    {
        size_t key;
        Value value;
        bool deleted;
    };


    struct HashMapKeyExistsResult
    {
        bool exists;
        size_t index;
    };

    enum HashMapErrors
    {
        HATESQL_HASHMAP_KEY_NOT_FOUND = -10,
    };

    template <typename Key, typename Value>
    class HashMap
    {
        Vector<HashMapData<Value>> vec;
        std::hash<Key> hash_func;
        size_t buffer_size;
    public:
        HashMap(size_t val_count_per_transfers = 5) {
            this->buffer_size = val_count_per_transfers;
        }

        // closes and opens db again
        int reopen() {
            return vec.reopen();
        }
        
        // open the database file
        int open(const std::string &file_name)
        {
            return vec.open(file_name);
        }

        // close the database file
        void close()
        {
            vec.close();
        }

        // insert key and value to the database
        int insert(const Key &key, const Value &value)
        {
            if (key_exists(key).exists) {
                return HATESQL_SUCCESS;
            }

            size_t hash_result = hash_func(key);
            size_t index = hash_result % (vec.size() + 1);
            HashMapData<Value> tmp = {hash_result, value , false};
            
            if (vec.size() != 0) {
                size_t set_index = hash_result % vec.size();
                HashMapData<Value> check_deleted;
                
                for (size_t i{set_index} ; i < vec.size() ; ++i) {
                    vec.get(i , check_deleted);



                    if (check_deleted.deleted) {
                        return vec.set(i , tmp);
                    }

                    if (check_deleted.key % vec.size() > set_index) {
                        break;
                    }
                }
            }


            return vec.buffered_insert(index, &tmp , 1 ,buffer_size);
        }

        // remove by key
        int remove(const Key &key)
        {
            auto check_key = key_exists(key);

            if (check_key.exists)
            {
                HashMapData<Value> del_val;
                vec.get(check_key.index , del_val);
                del_val.deleted = true;
                vec.set(check_key.index , del_val);
            }

            return HATESQL_HASHMAP_KEY_NOT_FOUND;
        }

        // checks if key exists , returns its index in HateSQL::Vector along with search result . it also remodifies the index of the key for faster searches
        HashMapKeyExistsResult key_exists(const Key &key)
        {

            if (vec.size() == 0) {
                return HashMapKeyExistsResult{false, 0};
            }

            size_t hash_result = hash_func(key);
            size_t index = (hash_func(key) % vec.size());

            HashMapData<Value> search_value;

            for (size_t i = index; i < vec.size(); ++i)
            {

                vec.get(i , search_value);

                
                if (search_value.key == hash_result)
                {
                    return {true, i};
                }
            }

            for (size_t i = 0; i < index; ++i)
            {
                vec.get(i , search_value);

                
                if (search_value.key == hash_result)
                {
                    HashMapData<Value> value_at_index;
                    vec.get(index , value_at_index);

                    size_t value_org_index = value_at_index.key % vec.size();

                    if (value_org_index <= index) {
                                            std::cout << "called ! 1\n";
                        vec.buffered_insert(index , &search_value , 1 , buffer_size);

                        search_value.deleted = true;
                        vec.set(i , search_value);
                    } else {
                                            std::cout << "called ! 2\n";
                        vec.set(index , search_value);
                        vec.set(i , value_at_index);
                    }



                    
                    return {true, index};
                }
            }

            return {false, 0};
        }

        // sets new value for the specific key
        int set(const Key &key, const Value &new_value)
        {
            auto check_key = key_exists(key);

            if (check_key.exists)
            {
                HashMapData<Value> map_result;
                vec.get(check_key.index , map_result);
                map_result.value = new_value;

                return vec.set(check_key.index , map_result);
            }

            return HATESQL_HASHMAP_KEY_NOT_FOUND;
        }

        // gets the value from db file and sets it to return_result
        int get(const Key &key, Value &return_result)
        {
            auto check_key = key_exists(key);

            if (check_key.exists)
            {
                HashMapData<Value> map_result;
                vec.get(check_key.index , map_result);

                return_result = map_result.value;

                return HATESQL_SUCCESS;
            }

            return HATESQL_HASHMAP_KEY_NOT_FOUND;
        }

        // get the HateSQL::Vector object which behind HateSQL::HashMap
        Vector<HashMapData<Value>> &get_the_underlying_vector()
        {
            return vec;
        }

        // cleanup
        ~HashMap()
        {
            vec.close();
        }
    };
}

#endif // HATE_SQL_HASHMAP
