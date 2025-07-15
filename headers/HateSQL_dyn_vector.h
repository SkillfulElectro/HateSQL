#ifndef HATE_SQL_DYN_VECTOR
#define HATE_SQL_DYN_VECTOR

#include "HateSQL.h"

namespace HateSQL {
    enum DynVectorError {
        HATESQL_DYN_VECTOR_INVALID_DATA_FILE_PATH = -20,
        HATESQL_DYN_VECTOR_NOT_DATA_FILE = -21,
        HATESQL_DYN_VECTOR_DATA_FILE_NOT_OPEN = -22,
        HATESQL_DYN_VECTOR_META_DATA_FILE_NOT_OPEN = -23,
        HATESQL_DYN_VECTOR_INVALID_VALUE_SIZE_PASSED = -24,
    };

    struct DynVectorData {
        size_t start_index;
        size_t len;
    };

    size_t hate_sql_data_file_code = std::hash<std::string>()("HateSQL_data_file");

    int data_file_exists(const std::string& file_path) {
        std::fstream file(file_path , std::ios::in | std::ios::binary);

        if (file.is_open()) {

            file.seekg(0, std::ios::beg);
            auto len = file.tellg();
            file.seekg(0, std::ios::end);
            len = file.tellg() - len;

            if (len == 0)
            {
                file.close();
                std::remove(file_path.c_str());
                return HATESQL_DYN_VECTOR_INVALID_DATA_FILE_PATH;
            }

            file.seekg(0 , std::ios::beg);
            size_t signature;
            file.read(reinterpret_cast<char *>(&signature) , sizeof(size_t));

            file.close();

            if (signature == hate_sql_data_file_code) {
                
                return HATESQL_SUCCESS;
            } else {
                return HATESQL_DYN_VECTOR_NOT_DATA_FILE;
            }
        }

        return HATESQL_DYN_VECTOR_INVALID_DATA_FILE_PATH;
    }


    class DynVector {
        Vector<DynVectorData> meta_data;
        std::fstream data_file;
        std::string data_file_name;
    public:
        int open(const std::string& meta_file_path , const std::string& data_file_path) {
            int open_meta_data_res = meta_data.open(meta_file_path);

            if (open_meta_data_res != HATESQL_SUCCESS) {
                return open_meta_data_res;
            }

            int data_file_op_res = data_file_exists(data_file_path);
            switch (data_file_op_res) {
            case HATESQL_DYN_VECTOR_INVALID_DATA_FILE_PATH:
                data_file.open(data_file_path , std::ios::out | std::ios::binary);
                data_file.close();
                data_file.open(data_file_path , std::ios::in | std::ios::out | std::ios::binary);
                
                data_file.seekp(0 , std::ios::beg);
                data_file.write(reinterpret_cast<const char*>(&hate_sql_data_file_code) , sizeof(size_t));

                break;
            case HATESQL_DYN_VECTOR_NOT_DATA_FILE:
                meta_data.close();
                return data_file_op_res;
            case HATESQL_SUCCESS :

                data_file.open(data_file_path , std::ios::in | std::ios::out | std::ios::binary);

                return HATESQL_SUCCESS;
            }            

            return HATESQL_SUCCESS;
        }
        
        int close() {
            data_file.close();
            meta_data.close();
            return HATESQL_SUCCESS;
        }


        int set(size_t index , const void* value , size_t value_size) {
            if (!data_file.is_open()) {
                return HATESQL_DYN_VECTOR_DATA_FILE_NOT_OPEN;
            }

            if (!meta_data.is_open()) {
                return HATESQL_DYN_VECTOR_META_DATA_FILE_NOT_OPEN;
            }

            DynVectorData loc;
            int result = meta_data.get(index , loc);

            if (result != HATESQL_SUCCESS) {
                return result;
            }

            if (loc.len >= value_size) {
                data_file.seekp(loc.start_index , std::ios::beg);
                data_file.write(reinterpret_cast<const char*>(value) , value_size);

                return HATESQL_SUCCESS;
            } else {
                data_file.seekp(0 , std::ios::end);
                loc.start_index = data_file.tellp();
                loc.len = value_size;

                data_file.write(reinterpret_cast<const char*>(value) , value_size);

                result = meta_data.set(index , loc);
            }

            return result;
        }

        
        int get(size_t index , void* value , size_t value_size) {
            if (!data_file.is_open()) {
                return HATESQL_DYN_VECTOR_DATA_FILE_NOT_OPEN;
            }

            if (!meta_data.is_open()) {
                return HATESQL_DYN_VECTOR_META_DATA_FILE_NOT_OPEN;
            }


            DynVectorData loc;
            int result = meta_data.get(index , loc);

            if (result != HATESQL_SUCCESS) {
                return result;
            }

            if (loc.len == value_size) {
                data_file.seekg(loc.start_index , std::ios::beg);
                data_file.read(reinterpret_cast<char*>(value) , loc.len);

                return HATESQL_SUCCESS;
            } else {
                return HATESQL_DYN_VECTOR_INVALID_VALUE_SIZE_PASSED;
            }
        }


        int push_back(const void* value , size_t value_size) {
            if (!meta_data.is_open()) {
                return HATESQL_DYN_VECTOR_META_DATA_FILE_NOT_OPEN;
            }

            DynVectorData loc;
            data_file.seekp(0 , std::ios::end);
            loc.start_index = data_file.tellp();
            loc.len = value_size;

            int result = meta_data.push_back(loc);

            if (result != HATESQL_SUCCESS) {
                return result;
            }

            return set(meta_data.size() - 1 , value , value_size);
        }

        int pop_back() {
            return meta_data.pop_back();
        }

        int erase(size_t index) {
            return meta_data.erase(index , index + 1);
        }


        int insert(size_t index , void* value , size_t value_size) {
            if (!meta_data.is_open()) {
                return HATESQL_DYN_VECTOR_META_DATA_FILE_NOT_OPEN;
            }

            DynVectorData loc;
            data_file.seekp(0 , std::ios::end);
            loc.start_index = data_file.tellp();
            loc.len = value_size;

            int result = meta_data.insert(index , loc);

            if (result != HATESQL_SUCCESS) {
                return result;
            }

            return set(index , value , value_size);
        }

        size_t size() {
            return meta_data.size();
        }

        ~DynVector() {
            close();
        }
    };
}

#endif