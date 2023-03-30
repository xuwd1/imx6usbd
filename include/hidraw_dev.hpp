#pragma once


// unix
#include <fcntl.h>
#include <sys/select.h>
#include <unistd.h>

//C
#include <cstdint>
#include <string>
#include <memory>
#include <cstring>
#include <stdexcept>
#include <cerrno>
#include <cstdio>


//custom
#include "cerr.hpp"
#include "buffer.hpp"





struct HIDWriteBuffer:public Buffer<false>{
    using Buffer::Buffer;
    template <typename T>
    uint8_t* make_buf(uint8_t report_id, const T& obj){
        enlarge(sizeof(T)+sizeof(report_id));
        _data[0] = report_id;
        memcpy(&_data[1], &obj, sizeof(T));
        return get_data_ptr();
    }
    uint8_t* make_buf(uint8_t report_id, const uint8_t* data, size_t data_len){
        enlarge(data_len+sizeof(report_id));
        _data[0] = report_id;
        memcpy(&_data[1], data, data_len);
        return get_data_ptr();
    }
    
};

struct HIDReadBuffer:public Buffer<false>{
    using Buffer::Buffer;
    uint8_t* prepare_buf(size_t readsize){
        enlarge(readsize);
        return get_data_ptr();
    }
    uint8_t get_report_id(){
        return _data[0];
    }
    uint8_t* get_obj_ptr(){
        return &_data[1];
    }
};


/*
    a lightweight hidraw interface wrapper class
*/
class HidrawDevice{
    public:
        HidrawDevice(const std::string hidraw_devnod_path):
        write_buffer(1025),
        read_buffer(1025)
        {
            fd = open(hidraw_devnod_path.c_str(),O_RDWR); // we always use blocked io
            if (fd<0){
                throw CErrException(errno);
            }
            
        }
        ~HidrawDevice(){
            if (fd>=0){
                close(fd);
            }
        }


        template <typename T>
        void write_obj(uint8_t report_id, const T& obj){
            static_assert(sizeof(T)<=1024,"writing obj with size greater than 1024 in a single hid report voilates hid protocol!");
            auto* wbuf = write_buffer.make_buf(report_id, obj);
            auto res = write(this->fd,wbuf,sizeof(obj));
            if (res<0) {
                throw CErrException(errno);
            }
        }

        void write_rawdata(uint8_t report_id, const uint8_t* data, size_t data_len){
            if (data_len > 1024){
                throw std::invalid_argument("writing obj with size greater than 1024 in a single hid report voilates hid protocol!");
            }
            auto* wbuf = write_buffer.make_buf(report_id,data,data_len);
            auto res = write(this->fd,wbuf,data_len+sizeof(report_id));
            if (res<0) {
                throw CErrException(errno);
            }
        }

        template <typename T>
        void read_obj_idchk(uint8_t report_id, T& obj){
            auto got_rid = read_obj(std::forward<T&>(obj));
            if (got_rid != report_id){
                throw std::runtime_error("got unexpected report id");
            }
        }

        template <typename T>
        uint8_t read_obj(T& obj){
            static_assert(std::is_trivial_v<T>,"reading into obj that is not trivial" );
            static_assert(sizeof(T)<=1024,"reading obj with size greater than 1024 in a single hid report voilates hid protocol!");
            uint8_t report_id;
            auto* rbuf = read_buffer.prepare_buf(sizeof(T)+sizeof(report_id));
            auto res = read(this->fd,rbuf,sizeof(T)+sizeof(report_id));
            if (res<0) {
                throw CErrException(errno);
            } 
            if (res-1 != sizeof(T)){
                printf("requested:%lu got:%lu\n",sizeof(T),res);
                throw std::invalid_argument("got unexpected amount of data");
            }
            memcpy(&obj, read_buffer.get_obj_ptr(), sizeof(T));
            report_id = read_buffer.get_report_id();
            return report_id;
        }

        void read_rawdata_idchk(uint8_t report_id, uint8_t* readout_data,size_t data_len){
            auto got_rid = read_rawdata(readout_data, data_len);
            if (got_rid != report_id){
                throw std::runtime_error("got unexpected report id");
            }
        }

        uint8_t read_rawdata(uint8_t* readout_data,size_t data_len){
            if (data_len > 1024){
                throw std::invalid_argument("writing obj with size greater than 1024 in a single hid report voilates hid protocol!");
            }
            uint8_t report_id;
            auto* rbuf = read_buffer.prepare_buf(data_len+sizeof(report_id));
            auto res = read(this->fd,rbuf,data_len+sizeof(report_id));
            if (res<0) {
                throw CErrException(errno);
            } 
            if (res-1 != data_len){
                printf("requested:%lu got:%lu\n",data_len,res);
                throw std::invalid_argument("got unexpected amount of data");
            }
            memcpy(readout_data, read_buffer.get_obj_ptr(), data_len);
            report_id = read_buffer.get_report_id();
            return report_id;
        }

        bool select_read(timeval* timeout){
            fd_set rfds;
            FD_ZERO(&rfds);
            FD_SET(fd,&rfds);
            auto res = select(5,&rfds,NULL,NULL,timeout);
            return res!=0;
        }

        bool select_write(timeval* timeout){
            fd_set wfds;
            FD_ZERO(&wfds);
            FD_SET(fd,&wfds);
            auto res = select(1,NULL,&wfds,NULL,timeout);
            return res!=0;
        }

        

    private:
        int fd;
        HIDWriteBuffer write_buffer;
        HIDReadBuffer read_buffer;
        //TODO: hidraw probe
};


