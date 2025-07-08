#ifndef HATE_SQL_VECTOR
#define HATE_SQL_VECTOR

#include <iostream>
#include <fstream>
#include <filesystem>
#include <cstdio>

namespace HateSQL {
    enum StackErrors {
        HATESQL_VECTOR_SUCCESS = 0,
        HATESQL_VECTOR_DB_IS_NOT_OPENED,
        HATESQL_VECTOR_NOT_VECTOR_FILE,
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

        int open(const std::string& file_name) {
            this->close();

            // check if db exists
            file = std::fstream(file_name , std::ios::binary | std::ios::in | std::ios::out);
            if (file.is_open()) {
                file.seekg(-sizeof(StackFooter) , std::ios::end);
                file.read(reinterpret_cast<char *>(&footer) , sizeof(StackFooter));
                file.close();

                if ((footer.file_type_name != 
                    ('H' + 'a' + 't' + 'e' + 's' + 'q' + 'l' + 'v' + 'e' + 'c')
                )) {
                    footer.len = 0;
                    return HATESQL_VECTOR_NOT_VECTOR_FILE;
                }

                std::filesystem::resize_file(file_name , footer.len * sizeof(Value));
                std::rename(file_name.c_str() , (file_name + ".body").c_str());
                file.open(file_name + ".body" , std::ios::binary | std::ios::in | std::ios::out);
            } else {
                file.open(file_name + ".body" , std::ios::binary | std::ios::out);
                file.close();
                file.open(file_name + ".body" , std::ios::binary | std::ios::in | std::ios::out);
                footer.len = 0;
                footer.file_type_name = ('H' + 'a' + 't' + 'e' + 's' + 'q' + 'l' + 'v' + 'e' + 'c');
            }

            
            this->file_name = file_name;

            return HATESQL_VECTOR_SUCCESS;
        }

        void close() {
            if (file.is_open()) {
                file.seekg(0 , std::ios::end);
                file.write(reinterpret_cast<const char*>(&footer) , sizeof(StackFooter));
                footer.len = 0;
                file.close();
                std::rename((file_name + ".body").c_str() , file_name.c_str());
            }
        }

        int push_back(const Value& value) {
            if (!file.is_open()) {
                return HATESQL_VECTOR_DB_IS_NOT_OPENED;
            }

            file.seekp(0 , std::ios::end);
            file.write(reinterpret_cast<const char*>(&value) , sizeof(Value));
            footer.len += 1;

            return HATESQL_VECTOR_SUCCESS;
        }

        int pop_back() {
            if (!file.is_open()) {
                return HATESQL_VECTOR_DB_IS_NOT_OPENED;
            }

            footer.len -= 1;
            file.close();
            std::filesystem::resize_file(file_name + ".body" , footer.len * sizeof(Value));

            file.open(file_name + ".body" , std::ios::binary | std::ios::in | std::ios::out);

            return HATESQL_VECTOR_SUCCESS;
        }

        Value at(size_t index) {
            if (!file.is_open()) {
                return HATESQL_VECTOR_DB_IS_NOT_OPENED;
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