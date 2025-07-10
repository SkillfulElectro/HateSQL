#ifndef HATE_SQL_VECTOR
#define HATE_SQL_VECTOR

#include <iostream>
#include <fstream>
#include <filesystem>
#include <cstdlib>

namespace HateSQL
{
    enum VectorErrors
    {
        HATESQL_VECTOR_SUCCESS = 0,
        HATESQL_VECTOR_DB_IS_NOT_OPENED = -1,
        HATESQL_VECTOR_NOT_VECTOR_FILE = -2,
        HATESQL_VECTOR_EXISTS = -3,
        HATESQL_VECTOR_DOES_NOT_EXISTS = -4,
        HATESQL_VECTOR_INVALID_INDEX_PASSED = -5,
    };

    struct VectorFooter
    {
        size_t len;
        size_t file_type_name;
    };

    template <typename Value>
    class Vector
    {
    protected:
        VectorFooter footer;
        std::fstream file;
        std::string file_name;

    public:
        Vector()
        {
            footer.len = 0;
        }

        // check if db file exists
        static int exists(const std::string &file_name)
        {
            auto tmp = std::fstream(file_name, std::ios::binary | std::ios::in | std::ios::out);
            VectorFooter tmp_footer;

            if (tmp.is_open())
            {
                tmp.seekg(0, std::ios::beg);
                auto len = tmp.tellg();
                tmp.seekg(0, std::ios::end);
                len = tmp.tellg() - len;

                if (len == 0)
                {
                    std::remove(file_name.c_str());
                    return HATESQL_VECTOR_DOES_NOT_EXISTS;
                }

                tmp.seekg(-sizeof(VectorFooter), std::ios::end);
                tmp.read(reinterpret_cast<char *>(&tmp_footer), sizeof(VectorFooter));
                tmp.close();

                if (tmp_footer.file_type_name != std::hash<std::string>()("HateSQL_vector_file"))
                {
                    return HATESQL_VECTOR_NOT_VECTOR_FILE;
                }

                return HATESQL_VECTOR_EXISTS;
            }

            return HATESQL_VECTOR_DOES_NOT_EXISTS;
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
                file.seekg(-sizeof(VectorFooter), std::ios::end);
                file.read(reinterpret_cast<char *>(&footer), sizeof(VectorFooter));
                file.close();

                std::filesystem::resize_file(file_name, footer.len * sizeof(Value));
                std::rename(file_name.c_str(), (file_name + ".body").c_str());
                file.open(file_name + ".body", std::ios::binary | std::ios::in | std::ios::out);
            }
            break;

            case HATESQL_VECTOR_DOES_NOT_EXISTS:
            {
                file.open(file_name + ".body", std::ios::binary | std::ios::out);
                file.close();
                file.open(file_name + ".body", std::ios::binary | std::ios::in | std::ios::out);
                footer.len = 0;
                footer.file_type_name = std::hash<std::string>()("HateSQL_vector_file");
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
                file.seekp(0, std::ios::end);
                file.write(reinterpret_cast<const char *>(&footer), sizeof(VectorFooter));
                footer.len = 0;
                file.close();
                std::rename((file_name + ".body").c_str(), file_name.c_str());
                footer.len = footer.file_type_name = 0;
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

            file.seekp(0, std::ios::end);
            file.write(reinterpret_cast<const char *>(values), sizeof(Value) * values_len);
            footer.len += values_len;

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

            footer.len -= pop_len;
            file.close();
            std::filesystem::resize_file(file_name + ".body", footer.len * sizeof(Value));

            file.open(file_name + ".body", std::ios::binary | std::ios::in | std::ios::out);

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

            HateSQL::Vector<Value> tmp;
            tmp.open(file_name + ".insert");


            Value* buffer = new Value[buffer_size];

            for (size_t i = index; i < size();)
            {
                size_t len = (buffer_size > size() - i) ? size() - i : buffer_size;
                buffered_get(i , buffer , len);
                tmp.buffered_push_back(buffer , len);

                i += len;
            }

            for (size_t i {size()}; i != index;)
            {
                size_t len = (buffer_size > i - index) ? i - index : buffer_size;
                buffered_pop_back(len);

                i -= len;
            }


            buffered_push_back(values , values_len);


            for (size_t i = 0; i < tmp.size();)
            {
                size_t len = (buffer_size > size() - i) ? size() - i : buffer_size;
                tmp.buffered_get(i , buffer , len);
                buffered_push_back(buffer , len);

                i += len;
            }


            tmp.close();
            delete[] buffer;

            std::remove((file_name + ".insert").c_str());

            return HATESQL_VECTOR_SUCCESS;
        }   

        // gets value with index and sets it to return_result
        int get(size_t index , Value& return_result)
        {
            Value result;

            if (!file.is_open())
            {


                return HATESQL_VECTOR_DB_IS_NOT_OPENED;
            }

            if (index > size()) {


                return HATESQL_VECTOR_INVALID_INDEX_PASSED;
            }
            

            file.seekg(index * sizeof(Value), std::ios::beg);
            file.read(reinterpret_cast<char *>(&return_result), sizeof(Value));

            return HATESQL_VECTOR_SUCCESS;
        }

        // sets new_value to index
        int set(size_t index , const Value& new_value) {
            if (!file.is_open())
            {
                return HATESQL_VECTOR_DB_IS_NOT_OPENED;
            }

            if (index > size()) {
                return HATESQL_VECTOR_INVALID_INDEX_PASSED;
            }


            file.seekp(index * sizeof(Value), std::ios::beg);
            file.write(reinterpret_cast<const char *>(&new_value), sizeof(Value));


            return HATESQL_VECTOR_SUCCESS;
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

            file.seekg(index * sizeof(Value), std::ios::beg);
            file.read(reinterpret_cast<char *>(buffer), sizeof(Value) * buffer_size);

            return HATESQL_VECTOR_SUCCESS;
        }

        // start from index and set buffer values
        int buffered_set(size_t index , Value* buffer , size_t buffer_size) {
            if (!file.is_open())
            {
                return HATESQL_VECTOR_DB_IS_NOT_OPENED;
            }

            if (index > size()) {
                return HATESQL_VECTOR_INVALID_INDEX_PASSED;
            }

            file.seekp(index * sizeof(Value), std::ios::beg);
            file.write(reinterpret_cast<const char *>(buffer), sizeof(Value) * buffer_size);


            return HATESQL_VECTOR_SUCCESS;
        }

        // returns len of the vector
        size_t size()
        {
            return footer.len;
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