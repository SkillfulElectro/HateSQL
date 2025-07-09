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

    protected:
        struct VectorValueKeeper
        {
            bool modified = false;
            size_t index = 0;
            Value result;
        };

        VectorValueKeeper return_result;

        /// writes changes to return_result to the db file
        void set_return_result_to_file()
        {
            if (!file.is_open())
            {
                return;
            }

            if (return_result.modified)
            {
                file.seekp(return_result.index * sizeof(Value), std::ios::beg);
                file.write(reinterpret_cast<const char *>(&return_result.result), sizeof(Value));
                return_result.modified = false;
            }
        }

    public:
        Vector()
        {
            footer.len = 0;
            return_result.modified = false;
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
                set_return_result_to_file();
                file.seekp(0, std::ios::end);
                file.write(reinterpret_cast<const char *>(&footer), sizeof(VectorFooter));
                footer.len = 0;
                file.close();
                std::rename((file_name + ".body").c_str(), file_name.c_str());
                footer.len = footer.file_type_name = 0;
            }
        }

        // reopen the db file -> writes changes to the file and opens it again
        void update_db()
        {
            close();
            open(file_name);
        }

        // pushes to the back of the vector
        int push_back(const Value &value)
        {
            if (!file.is_open())
            {
                return HATESQL_VECTOR_DB_IS_NOT_OPENED;
            }

            file.seekp(0, std::ios::end);
            file.write(reinterpret_cast<const char *>(&value), sizeof(Value));
            footer.len += 1;

            return HATESQL_VECTOR_SUCCESS;
        }

        // pops from back of the vector
        int pop_back()
        {
            if (!file.is_open())
            {
                return HATESQL_VECTOR_DB_IS_NOT_OPENED;
            }

            footer.len -= 1;
            file.close();
            std::filesystem::resize_file(file_name + ".body", footer.len * sizeof(Value));

            file.open(file_name + ".body", std::ios::binary | std::ios::in | std::ios::out);

            return HATESQL_VECTOR_SUCCESS;
        }

        // erases from index start to end
        int erase(size_t start, size_t end)
        {
            if (!file.is_open())
            {
                return HATESQL_VECTOR_DB_IS_NOT_OPENED;
            }

            HateSQL::Vector<Value> tmp;
            tmp.open(file_name + ".erase");

            for (size_t i = end; i < size(); ++i)
            {
                tmp.push_back(at_const(i));
            }

            for (; start != size();)
            {
                pop_back();
            }

            for (size_t i = 0; i < tmp.size(); ++i)
            {
                push_back(tmp.at_const(i));
            }

            tmp.close();

            std::remove((file_name + ".erase").c_str());

            return HATESQL_VECTOR_SUCCESS;
        }

        // insert an element at specific index
        int insert(size_t index, const Value &val)
        {
            if (!file.is_open())
            {
                return HATESQL_VECTOR_DB_IS_NOT_OPENED;
            }

            HateSQL::Vector<Value> tmp;
            tmp.open(file_name + ".insert");

            for (size_t i = index; i < size(); ++i)
            {
                tmp.push_back(at_const(i));
            }

            while (size() > index)
            {
                pop_back();
            }


            push_back(val);


            for (size_t i = 0; i < tmp.size(); ++i)
            {
                push_back(tmp.at_const(i));
            }


            tmp.close();

            std::remove((file_name + ".insert").c_str());

            return HATESQL_VECTOR_SUCCESS;
        }

        /// returns constant ref to value at specific index
        const Value &at_const(size_t index)
        {
            auto& val = at(index);
            return_result.modified = false;

            return val;
        }

        Value& at(size_t index) {
            if (!file.is_open())
            {
                std::cerr << "In SQLHate::Vector::at | SQLHate::Vector::at_const database file is not opened !\n";
                std::abort();
            }

            if (index > size()) {
                close();
                std::cerr << "In SQLHate::Vector::at | SQLHate::Vector::at_const invalid index passed !\n";
                std::abort();
            }

            set_return_result_to_file();

            file.seekg(index * sizeof(Value), std::ios::beg);
            file.read(reinterpret_cast<char *>(&return_result.result), sizeof(Value));

            return_result.modified = true;
            return_result.index = index;

            return return_result.result;
        }

        // returns len of the vector
        size_t size()
        {
            return footer.len;
        }

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