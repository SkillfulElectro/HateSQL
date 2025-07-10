#ifndef HATE_SQL_HASHMAP
#define HATE_SQL_HASHMAP

#include "HateSQL_vector.h"

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

    public:
        static int exists(const std::string &file_name) {
            return Vector<Value>::exists(file_name);
        }
        
        int open(const std::string &file_name)
        {
            return vec.open(file_name);
        }

        void close()
        {
            vec.close();
        }

        int insert(const Key &key, const Value &value)
        {
            if (key_exists(key).exists) {
                return HATESQL_VECTOR_SUCCESS;
            }

            size_t hash_result = hash_func(key);
            size_t index = 0;
            if (vec.size() != 0) {
                size_t index = hash_result % vec.size();
            }
            
            return vec.insert(index, {hash_result, value});
        }

        int remove(const Key &key)
        {
            auto check_key = key_exists(key);

            if (check_key.exists)
            {
                return vec.erase(check_key.index, check_key.index + 1);
            }

            return HATESQL_HASHMAP_KEY_NOT_FOUND;
        }

        HashMapKeyExistsResult key_exists(const Key &key)
        {
            size_t index = 0;

            if (vec.size() != 0)
            {
                size_t index = (hash_func(key) % vec.size());
            }

            for (size_t i = index; i < vec.size(); ++i)
            {
                HashMapData<Value> val;
                vec.get(i , val);

                
                if (val.key == hash_func(key))
                {
                    return {true, i};
                }
            }

            for (size_t i = 0; i < index; ++i)
            {
                HashMapData<Value> val;
                vec.get(i , val);

                
                if (val.key == hash_func(key))
                {
                    auto cp_val = val;
                    vec.erase(i , i + 1);
                    vec.insert(index , cp_val);
                    return {true, index};
                }
            }

            return {false, 0};
        }

        int set(const Key &key, const Value &value)
        {
            auto check_key = key_exists(key);

            if (check_key.exists)
            {
                HashMapData<Value> map_result;
                vec.get(check_key.index , map_result);
                map_result.value = value;

                return vec.set(check_key.index , map_result);
            }

            return HATESQL_HASHMAP_KEY_NOT_FOUND;
        }

        int get(const Key &key, Value &return_result)
        {
            auto check_key = key_exists(key);

            if (check_key.exists)
            {
                HashMapData<Value> map_result;
                vec.get(check_key.index , map_result);

                return_result = map_result.value;

                return HATESQL_VECTOR_SUCCESS;
            }

            return HATESQL_HASHMAP_KEY_NOT_FOUND;
        }

        Vector<HashMapData<Value>> &get_the_underlying_vector()
        {
            return vec;
        }

        ~HashMap()
        {
            vec.close();
        }
    };
}

#endif // HATE_SQL_HASHMAP
