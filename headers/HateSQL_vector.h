#ifndef HATE_SQL_VECTOR
#define HATE_SQL_VECTOR

#include <iostream>
#include <fstream>
#include <filesystem>
#include <cstdlib>

#include "HateSQL_utils.h"

namespace HateSQL
{
    template <typename Value>
    class Vector
    {
    protected:
        VectorHeader header;
        std::fstream file;
        std::string file_name;

    public:
        Vector()
        {
            header.len = 0;
        }

        // closes prev open file , and opens new one
        int open(const std::string &file_name)
        {
            this->close();

            switch (exists(file_name))
            {
            case HATESQL_VECTOR_EXISTS:
            {
                file.open(file_name, std::ios::binary | std::ios::in | std::ios::out);
                file.seekg(0 , std::ios::beg);
                file.read(reinterpret_cast<char *>(&header), sizeof(VectorHeader));

            }
            break;

            case HATESQL_VECTOR_DOES_NOT_EXISTS:
            {
                file.open(file_name , std::ios::binary | std::ios::out);
                file.close();
                file.open(file_name , std::ios::binary | std::ios::in | std::ios::out);
                header.len = 0;
                header.file_type_name = std::hash<std::string>()("HateSQL_vector_file");

                file.seekp(0 , std::ios::beg);
                file.write(reinterpret_cast<const char *>(&header), sizeof(VectorHeader));
            }
            break;

            default:
                return HATESQL_VECTOR_NOT_VECTOR_FILE;
            }

            this->file_name = file_name;

            return HATESQL_VECTOR_SUCCESS;
        }

        // writes changes to the db file and closes it
        void close()
        {
            if (file.is_open())
            {
                file.seekp(0 , std::ios::beg);
                file.write(reinterpret_cast<const char *>(&header), sizeof(VectorHeader));
                file.close();
                
                std::filesystem::resize_file(file_name , header.len * sizeof(Value) + sizeof(VectorHeader));

                header.len = 0;
                header.len = header.file_type_name = 0;
            }
        }

        // reopen the db file 
        int reopen()
        {
            close();
            return open(file_name);
        }

        // pushes value to the back of the vector
        int push_back(const Value &value)
        {
            return buffered_push_back(&value , 1);
        }

        // pushing multiple values to the back of the vector        
        int buffered_push_back(const Value* values , size_t values_len) {
            if (!file.is_open())
            {
                return HATESQL_VECTOR_DB_IS_NOT_OPENED;
            }

            size_t set_index = size();
            header.len += values_len;
            buffered_set(set_index , values , values_len);

            return HATESQL_VECTOR_SUCCESS;
        } 

        // pops value from back of the vector
        int pop_back()
        {
            return buffered_pop_back(1);
        }

        // poping multiple values from back of the vector
        int buffered_pop_back(size_t pop_len) {
            if (!file.is_open())
            {
                return HATESQL_VECTOR_DB_IS_NOT_OPENED;
            }

            header.len -= pop_len;

            return HATESQL_VECTOR_SUCCESS;
        }

        // erases values from index start to end
        int erase(size_t start, size_t end)
        {
            return buffered_erase(start , end , 1);
        }

        // erases values from index start to end , buffer_size is used for optimizing I/O ops
        int buffered_erase(size_t start , size_t end , size_t buffer_size = 5) {
            if (!file.is_open())
            {
                return HATESQL_VECTOR_DB_IS_NOT_OPENED;
            }

            if (buffer_size == 0) {
                return HATESQL_VECTOR_INVALID_BUFFER_SIZE_PASSED;
            }


            Value* buffer = new Value[buffer_size];

            size_t start_val_keeper = start;

            for (size_t i = end; i < size();)
            {
                size_t len = (buffer_size > size() - i) ? size() - i : buffer_size;
                buffered_get(i , buffer , len);
                buffered_set(start , buffer , len);

                start += len;
                i += len;
            }

            buffered_pop_back(end - start_val_keeper);


            delete[] buffer;

            return HATESQL_VECTOR_SUCCESS;
        }

        // insert an element to specific index
        int insert(size_t index, const Value &val)
        {
            return buffered_insert(index , &val , 1 , 1);
        }

        // insert multiple values to the specific index , buffer_size is used for optimizing I/O ops
        int buffered_insert(size_t index , const Value* values , size_t values_len ,size_t buffer_size = 5) {
            if (!file.is_open())
            {
                return HATESQL_VECTOR_DB_IS_NOT_OPENED;
            }

            if (values_len == 0) {
                return HATESQL_VECTOR_SUCCESS;
            }

            if (buffer_size == 0) {
                return HATESQL_VECTOR_INVALID_BUFFER_SIZE_PASSED;
            }

            Value* buffer = new Value[buffer_size];

            size_t prev_len = size();

            header.len += values_len;

            size_t new_len = size();

            for (; prev_len > index ;) {
                size_t len = (prev_len - index < buffer_size) ? prev_len - index : buffer_size;
                size_t get_index = prev_len - len; 
                size_t set_index = new_len - len;

                buffered_get(get_index , buffer , len);
                buffered_set(set_index , buffer , len);

                prev_len -= len;
                new_len -= len;
            }

            buffered_set(index , values , values_len);

            delete[] buffer;

            return HATESQL_VECTOR_SUCCESS;
        }   

        // gets value with index and sets it to return_result
        int get(size_t index , Value& return_result)
        {
            return buffered_get(index , &return_result , 1);
        }

        // sets new_value to index
        int set(size_t index , const Value& new_value) {
            return buffered_set(index , &new_value , 1);
        }

        // start from index and fill the buffer
        int buffered_get(size_t index , Value* buffer , size_t buffer_size) {
            if (!file.is_open())
            {
                return HATESQL_VECTOR_DB_IS_NOT_OPENED;
            }

            if (index > size()) {
                return HATESQL_VECTOR_INVALID_INDEX_PASSED;
            }

            if (index + buffer_size > size()) {
                return HATESQL_VECTOR_INVALID_BUFFER_SIZE_PASSED;
            }

            file.seekg(index * sizeof(Value) + sizeof(VectorHeader), std::ios::beg);
            file.read(reinterpret_cast<char *>(buffer), sizeof(Value) * buffer_size);

            return HATESQL_VECTOR_SUCCESS;
        }

        // start from index and set buffer values
        int buffered_set(size_t index ,const Value* values , size_t values_len) {
            if (!file.is_open())
            {
                return HATESQL_VECTOR_DB_IS_NOT_OPENED;
            }

            if (index > size()) {
                return HATESQL_VECTOR_INVALID_INDEX_PASSED;
            }

            if (index + values_len > size()) {
                return HATESQL_VECTOR_INVALID_BUFFER_SIZE_PASSED;
            }

            file.seekp(index * sizeof(Value) + sizeof(VectorHeader), std::ios::beg);
            file.write(reinterpret_cast<const char *>(values), sizeof(Value) * values_len);


            return HATESQL_VECTOR_SUCCESS;
        }

        // returns len of the vector
        size_t size()
        {
            return header.len;
        }

        // gets file name
        const std::string& get_file_name() {
            return file_name;
        }

        // cleanup
        ~Vector()
        {
            close();
        }
    };


};

#endif