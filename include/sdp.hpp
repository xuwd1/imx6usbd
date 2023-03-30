
#pragma once

// unix
#include <fcntl.h>
#include <unistd.h>

//C
#include <cstdint>
#include <string>
#include <cstdint>
#include <memory>
#include <stdexcept>

//custom
#include "cerr.hpp"
#include "hidraw_dev.hpp"
#include "buffer.hpp"

enum SDPCommandType:uint16_t{
    READ_REGISTER=0x0101,
    WRITE_REGISTER=0X0202,
    WRITE_FILE=0X0404,
    ERROR_STATUS=0X0505,
    DCD_WRITE=0X0A0A,
    JUMP_ADDRESS=0X0B0B,
    SKIP_DCD_HEADER=0X0C0C
    
};

enum SDPFormat:uint8_t{
    ACCESS_8BIT=0x8,
    ACCESS_16BIT=0x10,
    ACCESS_32BIT=0x20
};

#pragma pack(push, 1)
struct SDPCommand {
    SDPCommandType command_type;
    uint32_t addr;
    SDPFormat format;
    uint32_t data_count;
    uint32_t data;
    uint8_t _reserved{0};
};
#pragma pack(pop)


enum class SDPResponseType: uint32_t{
    kSecClose = 0x12343412,
    kSecOpen = 0x56787856,
    kWriteComplete = 0x128A8A12,
    kFileComplete = 0x88888888,
    kOkAck = 0x900DD009,
    kSingleValue = 0xDEADBEEF,
    kMultiValue = 0xBEEFDEAD

};

struct SDPResponse {
    private:
        SDPResponseType type;
        // this buffer's size should be strictly the same with how much data avaliable inside
        // that means, we ALWAYS assume that all data in it is useful
        Buffer<true> buffer;
    public:
    // constructor for single value response
    static SDPResponse make_single(uint32_t v_recvd){
        SDPResponse ret;
        switch (v_recvd){
            case static_cast<uint32_t>(SDPResponseType::kSecClose):
                ret.type = SDPResponseType::kSecClose;
                break;
            case static_cast<uint32_t>(SDPResponseType::kSecOpen):
                ret.type = SDPResponseType::kSecOpen;
                break;
            case static_cast<uint32_t>(SDPResponseType::kWriteComplete):
                ret.type = SDPResponseType::kWriteComplete;
                break;
            case static_cast<uint32_t>(SDPResponseType::kFileComplete):
                ret.type = SDPResponseType::kFileComplete;
                break;
            case static_cast<uint32_t>(SDPResponseType::kOkAck):
                ret.type = SDPResponseType::kOkAck;
                break;
            default:
                ret.type = SDPResponseType::kSingleValue;
                ret.buffer.enlarge(sizeof(uint32_t));
                auto* ptr = reinterpret_cast<uint32_t*>(ret.buffer.get_data_ptr()); 
                *ptr = v_recvd;
                break;
        }
        return ret;
    }
    static SDPResponse make_multi(size_t init_bufsize){
        SDPResponse ret;
        ret.type = SDPResponseType::kMultiValue;
        ret.buffer.enlarge(init_bufsize);
        return ret;
    }

    /*
        enlarge internal buffer and return pointer to the enlarged buffer 
    */
    uint8_t* enlarge_buffer(size_t new_bufsize){
        buffer.enlarge(new_bufsize);
        return buffer.get_data_ptr();
    }

    // SDPResponse data must be obtained from operator[]
    const uint32_t operator[](size_t idx){
        switch (type) {
            case SDPResponseType::kSingleValue:{
                auto* ptr = reinterpret_cast<uint32_t*>(buffer.get_data_ptr()); 
                return *ptr;
            }
                
            case SDPResponseType::kMultiValue:{
                if (!(idx<buffer.cur_buffer_size)){
                    throw std::invalid_argument("index out of range");
                }
                auto* ptr = reinterpret_cast<uint32_t*>(buffer.get_data_ptr()); 
                return ptr[idx];
            }

            default:{
                if (!(idx==0)){
                    throw std::invalid_argument("index out of range");
                }
                return static_cast<uint32_t>(type);
            }

        }
    }
};



class SDPDevice{
    
    public:
        SDPDevice(const std::string& hidraw_devnod_path):
        hidraw_device(hidraw_devnod_path){}
        /*
            Methods for doing SDP TRANSACTIONS
            By TRANSACTION we mean a certain job consisted of several hid reports,
                and each Transaction corresponds to exactly one SDP command
            
            For example read_register transaction has tree hid reports:
                1. report 1 for comman "READ_REGISTER"
                2. report 3 for secruity config
                3. report 4 for reg value
        */ 


        /*
        Transaction for reading a single register
        This is recommended over "read_registers", which reads multiple registers in one transaction.
        only reg_addr should be provided since :
        (1) access format is ignored in this transaction from experiments
        (2) data count is set to be 4
        */
        SDPResponse read_register(uint32_t reg_addr);
        SDPResponse read_registers(uint32_t reg_addr, uint32_t data_count);
        SDPResponse write_register(uint32_t reg_addr, SDPFormat format, uint32_t data);
        SDPResponse write_file(uint32_t dst_addr, uint8_t* filebuf, uint32_t len);
        
        /*
        Transaction for writing into imx6, and EXECUTING dcd commands written
        normally the temp_dcd_addr should be in OCRAM, and the dcd commands sent should initialize DDR controller
        */
        SDPResponse dcd_write(uint32_t temp_dcd_addr, uint8_t* dcdbuf, uint32_t len);
        
        /*
        Transaction for skipping execute dcd commands inside the downloaded image 
        there is a TYPO in IMX6 reference
        this transaction should be commenced BEFORE JUMPING and after DCD_WRITE 
        */
        SDPResponse skip_dcd();
        
        /*
        Transaction for jumping to IMAGE HEADER ADDR
        YES THIS IS RIGHT, in SDP the dst addr of jump should be the image addr
        instead of executable addr.
        */
        SDPResponse jump_address(uint32_t dst_addr);

        
        

        /*
        Generic send_command (hid report 1)
        The endianess of various fields inside SDPCommand "cmd" 
        will be properly adjusted in this method
        so just simply pass little-endian cmd here
        */
        void send_command(const SDPCommand& cmd);
    
        /*
        Generic send_data (hid report 2)
        The method is used for sending device data associated with report 1
        NOTE THAT for a single report, data_len must be <= 1024
        data with data_len greater than 1024 will be automatically sent in batches
        */
        void send_data(const uint8_t* input_data,size_t data_len);

        template <typename T>
        void send_data(const T& obj);

        // generic get_response (hid report 3/4)
        SDPResponse get_response(uint8_t report_id,size_t response_data_len);

    private:
        HidrawDevice hidraw_device;

        //TODO: SDP probe

};