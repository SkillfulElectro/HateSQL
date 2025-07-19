#ifndef HATE_SQL
#define HATE_SQL

#include <iostream>
#include <fstream>
#include <cstdlib>

namespace HateSQL {
    enum HateSQLError {
        HATESQL_SUCCESS = 0,
        HATESQL_INVALID_METHOD = -1000,
    };

    template <typename Key , typename Value>
    class HateSQLBase {
    public:
        virtual int open(const std::string&) = 0;
        virtual void set_seek_start(const size_t&) = 0;
        virtual void set_buffer_size(const size_t&) = 0;
        virtual void close() = 0;
        virtual int insert(const Key&, const Value&) = 0;
        virtual bool is_open() = 0;
        virtual size_t size() = 0;
        virtual void clear() = 0;
        virtual int push_back(const Value&) {
            return HATESQL_INVALID_METHOD;
        }

        virtual int pop_back() {
            return HATESQL_INVALID_METHOD;
        }
        virtual int remove(const Key&) = 0;
        virtual int get(const Key&  , Value& ) = 0;
        virtual int set(const Key& , const Value&) = 0;
    };
}

#include "HateSQL_vector.h"
#include "HateSQL_hashmap.h"
#include "HateSQL_dyn.h"


#endif