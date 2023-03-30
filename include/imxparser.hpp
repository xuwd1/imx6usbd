#pragma once

//unix
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>

//C
#include <string>
#include <cerrno>
#include <cstdint>
#include <stdexcept>

//custom 
#include <cerr.hpp>

// solve clangd bug 
static_assert(true, "");

#pragma pack(push, 1)

struct IMxIVT{
    uint8_t tag;
    uint16_t length;
    uint8_t version;
    uint32_t entry;
    uint32_t reserved1;
    uint32_t dcd_ptr;
    uint32_t boot_data_ptr;
    uint32_t self;
    uint32_t csf_ptr;
    uint32_t reserved2;
};

struct IMxDCDHeader{
    uint8_t tag;
    uint16_t length;
    uint8_t version;
};


struct IMxBootData{
    uint32_t start;
    uint32_t length;
    uint32_t plugin;
};

#pragma pack(pop)

size_t locate_ivt(uint8_t* buf, size_t buflen);
bool validate_dcd(uint8_t* dcd);

struct IMxImageParseResult{
    // ivt info
    size_t ivt_offset;
    uint16_t ivt_size;

    // entry info
    uint32_t entry_addr;

    size_t dcd_offset;
    uint16_t dcd_size;

    size_t boot_data_offset;
    uint32_t image_load_addr; //parsed from boot_data, need to download file at this addr

};

class IMxImage{
public:
    IMxImage(const std::string& filepath){
        fd = open(filepath.c_str(),O_RDONLY);
        if (fd<0){
            throw CErrException(errno);
        }
        struct stat stat_buf;
        fstat(fd,&stat_buf);
        filesize = stat_buf.st_size;
        file_mmap = reinterpret_cast<uint8_t*>(mmap(0,filesize,PROT_READ,MAP_PRIVATE,fd,0));
        if (file_mmap==MAP_FAILED){
            throw CErrException(errno);
        }
    }
    ~IMxImage(){
        if (fd>=0){
            munmap(file_mmap,filesize);
            close(fd);
        }
    }
    uint8_t* get_file_buf(){
        return file_mmap;
    }
    size_t get_file_size(){
        return filesize;
    }
    IMxImageParseResult parse();

private:
    int fd;
    size_t filesize;
    uint8_t* file_mmap;
    
};