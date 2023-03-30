#pragma once

#include <exception>
#include <cstring>

struct CErrException : public std::exception
{
    CErrException(const int err_no):err_no(err_no){}
    const char * what () const throw ()
    {
        return strerror(this->err_no);
    }
    int err_no;
};

