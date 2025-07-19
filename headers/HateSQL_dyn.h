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
        HATESQL_DYN_VECTOR_NOT_POSSIBLE_TO_APPEND = -25,
        HATESQL_DYN_VECTOR_INVALID_OFFSET_OR_VALUE_SIZE_PASSED = -26,
    };

    struct DynVectorData {
        size_t start_index;
        size_t len;
    };

    size_t hate_sql_data_file_code = std::hash<std::string>()("HateSQL_data_file");

    int data_file_exists(const std::string& file_path , const size_t& seek_start = 0) {
        std::fstream file(file_path , std::ios::in | std::ios::binary);

        if (file.is_open()) {

            file.seekg(seek_start, std::ios::beg);
            auto len = file.tellg();
            file.seekg(0, std::ios::end);
            len = file.tellg() - len;

            if (len == 0)
            {
                file.close();
                std::remove(file_path.c_str());
                return HATESQL_DYN_VECTOR_INVALID_DATA_FILE_PATH;
            }

            file.seekg(seek_start , std::ios::beg);
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

    template <typename Key>
    class DynHashMap {
    protected:
        HateSQLBase<Key , DynVectorData>* meta_data;
        DynHashMap(HateSQLBase<Key , DynVectorData>* mt_dt) {
            meta_data = mt_dt;
            data_file_seek_start = 0;
        }
    private:
        std::fstream data_file;
        std::string data_file_name;
        size_t data_file_seek_start;
    public:
        DynHashMap() {
            meta_data = new HashMap<Key , DynVectorData>;
            data_file_seek_start = 0;
        }

        void set_buffer_size(size_t buffer_size) {
            meta_data->set_buffer_size(buffer_size);
        }

        void set_meta_file_seek_start(const size_t& seek_start) {
            meta_data->set_seek_start(seek_start);
        }

        void set_data_file_seek_start(const size_t& seek_start) {
            data_file_seek_start = seek_start;
        }

        // only possible on last index inserted or pushed back for now ! it appends the data at end of the prev passed data .
        int write_chunk_to(const Key& index , const void* value , const size_t& value_size) {
            DynVectorData data;
            int result = meta_data->get(index , data);

            if (result != HATESQL_SUCCESS) {
                return result;
            }

            size_t data_final_index = data_file_seek_start + data.start_index + data.len;
            data_file.seekp(0 , std::ios::end);

            if (data_final_index < data_file.tellp()) {
                return HATESQL_DYN_VECTOR_NOT_POSSIBLE_TO_APPEND;
            }

            data_file.write(reinterpret_cast<const char*>(value) , value_size);
            data.len += value_size;

            result = meta_data->set(index , data);

            return result;
        }

        int read_chunk_from(
            const Key& index 
            , void* value 
            , const size_t& offset_in_index
            , const size_t& value_size
        ) {
            
            DynVectorData data;
            int result = meta_data->get(index , data);

            if (result != HATESQL_SUCCESS) {
                return result;
            }

            if (data.start_index + data.len < data.start_index + offset_in_index + value_size) {
                return HATESQL_DYN_VECTOR_INVALID_OFFSET_OR_VALUE_SIZE_PASSED;
            }
            
            data_file.seekg(data_file_seek_start + data.start_index + offset_in_index , std::ios::beg);
            data_file.read(reinterpret_cast<char *>(value) , value_size);

            return HATESQL_SUCCESS;
        }

        int open(const std::string& meta_file_path , const std::string& data_file_path) {
            int open_meta_data_res = meta_data->open(meta_file_path);

            if (open_meta_data_res != HATESQL_SUCCESS) {
                return open_meta_data_res;
            }

            int data_file_op_res = data_file_exists(data_file_path , data_file_seek_start);
            switch (data_file_op_res) {
            case HATESQL_DYN_VECTOR_INVALID_DATA_FILE_PATH:
                data_file.open(data_file_path , std::ios::out | std::ios::binary);
                data_file.close();
                data_file.open(data_file_path , std::ios::in | std::ios::out | std::ios::binary);
                
                data_file.seekp(data_file_seek_start , std::ios::beg);
                data_file.write(reinterpret_cast<const char*>(&hate_sql_data_file_code) , sizeof(size_t));

                break;
            case HATESQL_DYN_VECTOR_NOT_DATA_FILE:
                meta_data->close();
                return data_file_op_res;
            case HATESQL_SUCCESS :

                data_file.open(data_file_path , std::ios::in | std::ios::out | std::ios::binary);

                return HATESQL_SUCCESS;
            }            

            return HATESQL_SUCCESS;
        }
        
        int close() {
            if (data_file.is_open()) {
                data_file.close();
            }
            meta_data->close();
            return HATESQL_SUCCESS;
        }


        int set(const Key& index , const void* value , size_t value_size) {
            if (!data_file.is_open()) {
                return HATESQL_DYN_VECTOR_DATA_FILE_NOT_OPEN;
            }

            if (!meta_data->is_open()) {
                return HATESQL_DYN_VECTOR_META_DATA_FILE_NOT_OPEN;
            }

            DynVectorData loc;
            int result = meta_data->get(index , loc);

            if (result != HATESQL_SUCCESS) {
                return result;
            }

            if (loc.len >= value_size) {
                data_file.seekp(data_file_seek_start + loc.start_index , std::ios::beg);
                data_file.write(reinterpret_cast<const char*>(value) , value_size);

                return HATESQL_SUCCESS;
            } else {
                data_file.seekp(0 , std::ios::end);
                loc.start_index = data_file.tellp() - data_file_seek_start;
                loc.len = value_size;

                data_file.write(reinterpret_cast<const char*>(value) , value_size);

                result = meta_data->set(index , loc);
            }

            return result;
        }

        
        int get(const Key& index , void* value , size_t value_size) {
            if (!data_file.is_open()) {
                return HATESQL_DYN_VECTOR_DATA_FILE_NOT_OPEN;
            }

            if (!meta_data->is_open()) {
                return HATESQL_DYN_VECTOR_META_DATA_FILE_NOT_OPEN;
            }


            DynVectorData loc;
            int result = meta_data->get(index , loc);

            if (result != HATESQL_SUCCESS) {
                return result;
            }

            if (loc.len == value_size) {
                data_file.seekg(data_file_seek_start + loc.start_index , std::ios::beg);
                data_file.read(reinterpret_cast<char*>(value) , loc.len);

                return HATESQL_SUCCESS;
            } else {
                return HATESQL_DYN_VECTOR_INVALID_VALUE_SIZE_PASSED;
            }
        }


        int push_back(const void* value , size_t value_size) {
            if (!meta_data->is_open()) {
                return HATESQL_DYN_VECTOR_META_DATA_FILE_NOT_OPEN;
            }

            DynVectorData loc;
            data_file.seekp(0 , std::ios::end);
            loc.start_index = data_file.tellp();
            loc.len = value_size;

            int result = meta_data->push_back(loc);

            if (result != HATESQL_SUCCESS) {
                return result;
            }

            return set(meta_data->size() - 1 , value , value_size);
        }

        int pop_back() {
            return meta_data->pop_back();
        }

        int remove(const Key& index) {
            return meta_data->remove(index);
        }


        int insert(const Key& index , void* value , size_t value_size) {
            if (!meta_data->is_open()) {
                return HATESQL_DYN_VECTOR_META_DATA_FILE_NOT_OPEN;
            }

            DynVectorData loc;
            data_file.seekp(0 , std::ios::end);
            loc.start_index = data_file.tellp();
            loc.len = value_size;

            int result = meta_data->insert(index , loc);

            if (result != HATESQL_SUCCESS) {
                return result;
            }

            return set(index , value , value_size);
        }

        void clear() {
            meta_data->clear();
        }

        int is_open() {
            if (!meta_data->is_open()) {
                return HATESQL_DYN_VECTOR_META_DATA_FILE_NOT_OPEN;
            }

            if (!data_file.is_open()) {
                return HATESQL_DYN_VECTOR_DATA_FILE_NOT_OPEN;
            }

            return HATESQL_SUCCESS;
        }

        size_t size() {
            return meta_data->size();
        }

        ~DynHashMap() {
            close();
            delete meta_data;
        }
    };


    class DynVector : public DynHashMap<size_t> {
        Vector<DynVectorData>* vec;
        Vector<DynVectorData>* initer() {
            vec = new Vector<DynVectorData>;

            return vec;
        }
    public:
        DynVector() : DynHashMap(initer()) {}
    };
}

#endif