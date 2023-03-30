#pragma once


//C
#include <cstdint>
#include <memory>
#include <cstring>
#include <cstdio>


template <bool cpy=true>
struct Buffer{
    Buffer(){
        _data.reset(new uint8_t[0]); 
        cur_buffer_size=0;
    }
    Buffer(size_t init_size){
        _data.reset(new uint8_t[init_size]); 
        cur_buffer_size=init_size;
    }
    void enlarge(size_t new_size){
        if (cur_buffer_size>=new_size){
            return;
        }
        auto new_buffer = new uint8_t[new_size];
        if constexpr (cpy){
            memcpy(new_buffer,_data.get(),cur_buffer_size);
        }
        _data.reset(new_buffer);
        cur_buffer_size = new_size;
    }
    uint8_t* get_data_ptr(){
        return _data.get();
    }
    std::unique_ptr<uint8_t[]> _data;
    size_t cur_buffer_size;

    void print_bytes(size_t num){
        for (size_t i=0;i<num;i++){
            printf("%hhX ",_data[i]);
        }
        printf("\n");
    }
};