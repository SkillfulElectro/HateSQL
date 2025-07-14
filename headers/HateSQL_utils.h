#ifndef HATE_SQL_UTILS
#define HATE_SQL_UTILS

namespace HateSQL {
    enum VectorErrors
    {
        HATESQL_VECTOR_SUCCESS = 0,
        HATESQL_VECTOR_DB_IS_NOT_OPENED = -1,
        HATESQL_VECTOR_NOT_VECTOR_FILE = -2,
        HATESQL_VECTOR_EXISTS = -3,
        HATESQL_VECTOR_DOES_NOT_EXISTS = -4,
        HATESQL_VECTOR_INVALID_INDEX_PASSED = -5,
        HATESQL_VECTOR_INVALID_BUFFER_SIZE_PASSED = -6,
    };

    struct VectorHeader
    {
        size_t len;
        size_t file_type_name;
    };

    // checks if a file is HateSQL::Vector
    int exists(const std::string &file_name)
    {
        auto tmp = std::fstream(file_name, std::ios::binary | std::ios::in | std::ios::out);
        VectorHeader tmp_header;

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

            tmp.seekg(0 , std::ios::beg);
            tmp.read(reinterpret_cast<char *>(&tmp_header), sizeof(VectorHeader));
            tmp.close();

            if (tmp_header.file_type_name != std::hash<std::string>()("HateSQL_vector_file"))
            {
                return HATESQL_VECTOR_NOT_VECTOR_FILE;
            }

            return HATESQL_VECTOR_EXISTS;
        }

        return HATESQL_VECTOR_DOES_NOT_EXISTS;
    }

    template <typename Value>
    // used for re calc the len in VectorHeader part of the file
    int update_header_len(const std::string &file_name) {
        if (exists(file_name) == HATESQL_VECTOR_EXISTS) {
            std::fstream file;
            VectorHeader header;

            file.open(file_name, std::ios::binary | std::ios::in | std::ios::out);
            file.seekg(0 , std::ios::beg);
            file.read(reinterpret_cast<char *>(&header), sizeof(VectorHeader));

            file.seekg(sizeof(VectorHeader) , std::ios::beg);
            size_t data_len = file.tellg();
            file.seekg(0 , std::ios::end);
            data_len = file.tellg() - data_len;

            header.len = data_len / sizeof(Value);

            file.seekp(0 , std::ios::beg);
            file.write(reinterpret_cast<const char *>(&header), sizeof(VectorHeader));

            return HATESQL_VECTOR_SUCCESS;
        }

        return HATESQL_VECTOR_NOT_VECTOR_FILE;
    }
}

#endif