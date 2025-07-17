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
        HATESQL_HASHMAP_FAILED_TO_INSERT = -11,
    };

    template <typename Key, typename Value>
    class HashMap : public HateSQLBase<Key , Value>
    {
        Vector<HashMapData<Value>> vec;
        std::hash<Key> hash_func;
        size_t buffer_size;
        
        size_t filled_len;
        double load_factor;
    private:
        void rehash() {
            Vector<HashMapData<Value>> tmp;
            std::string orginal_filename = vec.get_file_name();
            tmp.open(orginal_filename + ".rehash");

            size_t new_vec_size = vec.size() * 2;

            HashMapData<Value> filler_value;
            filler_value.deleted = true;
            for (size_t i {0} ; i < new_vec_size ; ++i) {
                tmp.push_back(filler_value);
            }

            size_t filled_count = 0;
            
            HashMapData<Value> tmp_value;
            for (size_t i{0} ; i < vec.size() ; ++i) {
                vec.get(i , filler_value);

                if (!filler_value.deleted) {
                    size_t new_index = filler_value.key % new_vec_size;

                    bool found_place = false;

                    for (size_t i{new_index} ; i < tmp.size() ; ++i) {
                        tmp.get(i , tmp_value);

                        if (tmp_value.deleted) {
                            found_place = true;
                            tmp.set(i , filler_value);
                            break;
                        }
                    }

                    if (!found_place) {
                        for (size_t i{0}; i < new_index ; ++i) {
                            tmp.get(i , tmp_value);

                            if (tmp_value.deleted) {
                                found_place = true;
                                tmp.set(i , filler_value);
                                break;
                            }
                        }
                    }
                    

                    filled_count += 1;
                }
            }

            tmp.close();
            vec.close();
            
            std::remove(orginal_filename.c_str());
            std::rename((orginal_filename + ".rehash").c_str() , orginal_filename.c_str());

            vec.open(orginal_filename);
            filled_len = filled_count;
        }

    public:
        HashMap(size_t val_count_per_transfers = 5) {
            this->buffer_size = val_count_per_transfers;
            load_factor = 0.75;
            filled_len = 0;
        }

        // closes and opens db again
        int reopen() {
            return vec.reopen();
        }
        
        // open the database file
        int open(const std::string &file_name) override 
        {
            int result = vec.open(file_name);

            if (result != HATESQL_SUCCESS) {
                return result;
            }

            HashMapData<Value> tmp;
            if (vec.size() == 0) {                
                tmp.deleted = false;
                tmp.key = 0;
                filled_len += 1;
                result = vec.push_back(tmp);
            } else {
                vec.get(0 , tmp);
                filled_len = tmp.key;
            }
            
            return result;
        }

        // close the database file
        void close() override 
        {
            HashMapData<Value> tmp;
            tmp.key = filled_len;
            vec.set(0 , tmp);
            vec.close();
        }

        // insert key and value to the database
        int insert(const Key &key, const Value &value) override
        {
            if (key_exists(key).exists) {
                return HATESQL_SUCCESS;
            }

            size_t hash_result = hash_func(key);
            HashMapData<Value> tmp = {hash_result, value , false};


            if ((double)filled_len / vec.size() >= load_factor) {
                rehash();
            }

            size_t index = hash_result % (vec.size());

            HashMapData<Value> check_value;
            bool found_place = false;

            for (size_t i{index}; i < vec.size() ; ++i) {
                vec.get(i , check_value);

                if (check_value.deleted) {

                    filled_len += 1;
                    found_place = true;
                    vec.set(i , tmp);
                    break;
                }

            }

            if (!found_place) {
                for (size_t i{1}; i < index ; ++i) {
                    vec.get(i , check_value);

                    if (check_value.deleted) {

                        filled_len += 1;
                        found_place = true;
                        vec.set(i , tmp);
                        break;
                    }

                }
            }



            return HATESQL_HASHMAP_FAILED_TO_INSERT;
        }

        // remove by key
        int remove(const Key &key) override
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
            size_t index = (hash_result % vec.size());

            HashMapData<Value> search_value;

            for (size_t i = index; i < vec.size(); ++i)
            {

                vec.get(i , search_value);

                
                if (search_value.key == hash_result)
                {
                    return {true, i};
                }
            }

            for (size_t i = 1; i < index; ++i)
            {

                vec.get(i , search_value);

                
                if (search_value.key == hash_result)
                {
                    return {true, i};
                }
            }

            return {false, 0};
        }

        // sets new value for the specific key
        int set(const Key &key, const Value &new_value) override 
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
        int get(const Key &key, Value &return_result) override 
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


        bool is_open() override {
            return vec.is_open();
        }

        size_t size() override {
            return filled_len - 1;
        }

        // cleanup
        ~HashMap()
        {
            vec.close();
        }
    };
}

#endif // HATE_SQL_HASHMAP
