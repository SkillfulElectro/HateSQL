#ifndef HATE_SQL_VECTOR
#define HATE_SQL_VECTOR

#include <iostream>
#include <fstream>
#include <filesystem>
#include <cstdio>

namespace HateSQL {
    enum StackErrors {
        HATESQL_STACK_SUCCESS = 0,
        HATESQL_STACK_DB_IS_NOT_OPENED,
        HATESQL_STACK_NOT_STACK_FILE,
        HATESQL_STACK_EXISTS,
        HATESQL_STACK_DOES_NOT_EXISTS,
    };

    struct StackFooter {
        size_t len;
        int file_type_name;
    };

    template <typename Value>
    class Stack {
        StackFooter footer;
        std::fstream file;
        std::string file_name;
    public:
        Stack() {
            footer.len = 0;
        }
        
        int exists(const std::string& file_name) {
            auto tmp = std::fstream(file_name , std::ios::binary | std::ios::in | std::ios::out);
            StackFooter tmp_footer;

            if (tmp.is_open()) {
                tmp.seekg(0 , std::ios::beg);
                auto len = tmp.tellg();
                tmp.seekg(0 , std::ios::end);
                len = tmp.tellg() - len;

                if (len == 0) {
                    std::remove(file_name.c_str());
                    return HATESQL_STACK_DOES_NOT_EXISTS;
                }

                tmp.seekg(-sizeof(StackFooter) , std::ios::end);
                tmp.read(reinterpret_cast<char *>(&tmp_footer) , sizeof(StackFooter));
                tmp.close();

                if ((tmp_footer.file_type_name != 
                    ('H' + 'a' + 't' + 'e' + 's' + 'q' + 'l' + 'v' + 'e' + 'c')
                )) {
                    return HATESQL_STACK_NOT_STACK_FILE;
                }

                return HATESQL_STACK_EXISTS;
            }

            return HATESQL_STACK_DOES_NOT_EXISTS;
        }

        int open(const std::string& file_name) {
            this->close();

            switch (exists(file_name)) {
            case HATESQL_STACK_EXISTS :
            {
                file.open(file_name , std::ios::binary | std::ios::in | std::ios::out);
                file.seekg(-sizeof(StackFooter) , std::ios::end);
                file.read(reinterpret_cast<char *>(&footer) , sizeof(StackFooter));
                file.close();

                std::filesystem::resize_file(file_name , footer.len * sizeof(Value));
                std::rename(file_name.c_str() , (file_name + ".body").c_str());
                file.open(file_name + ".body" , std::ios::binary | std::ios::in | std::ios::out);
            }
            break;

            case HATESQL_STACK_DOES_NOT_EXISTS :
            {
                file.open(file_name + ".body" , std::ios::binary | std::ios::out);
                file.close();
                file.open(file_name + ".body" , std::ios::binary | std::ios::in | std::ios::out);
                footer.len = 0;
                footer.file_type_name = ('H' + 'a' + 't' + 'e' + 's' + 'q' + 'l' + 'v' + 'e' + 'c');
            }
            break;

            default :
                return HATESQL_STACK_NOT_STACK_FILE;
            }

            
            this->file_name = file_name;

            return HATESQL_STACK_SUCCESS;
        }

        void close() {
            if (file.is_open()) {
                file.seekp(0 , std::ios::end);
                file.write(reinterpret_cast<const char*>(&footer) , sizeof(StackFooter));
                footer.len = 0;
                file.close();
                std::rename((file_name + ".body").c_str() , file_name.c_str());
                footer.len = footer.file_type_name = 0;
            }
        }

        int push_back(const Value& value) {
            if (!file.is_open()) {
                return HATESQL_STACK_DB_IS_NOT_OPENED;
            }

            file.seekp(0 , std::ios::end);
            file.write(reinterpret_cast<const char*>(&value) , sizeof(Value));
            footer.len += 1;

            return HATESQL_STACK_SUCCESS;
        }

        int pop_back() {
            if (!file.is_open()) {
                return HATESQL_STACK_DB_IS_NOT_OPENED;
            }

            footer.len -= 1;
            file.close();
            std::filesystem::resize_file(file_name + ".body" , footer.len * sizeof(Value));

            file.open(file_name + ".body" , std::ios::binary | std::ios::in | std::ios::out);

            return HATESQL_STACK_SUCCESS;
        }

        Value at(size_t index) {
            if (!file.is_open()) {
                return HATESQL_STACK_DB_IS_NOT_OPENED;
            }

            Value result;

            file.seekg(index * sizeof(Value) , std::ios::beg);
            file.read(reinterpret_cast<char*>(&result) , sizeof(Value));

            return result;
        }

        size_t size() {
            return footer.len;
        }

        ~Stack() {
            close();
        }
    };
};

#endif