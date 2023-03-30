
// unix

#include <cstdio>
#include <exception>
#include <fcntl.h>
#include <stdexcept>
#include <sys/types.h>
#include <unistd.h>

//C
#include <cerrno>
#include <cstdint>
#include <type_traits>

//custom
#include "sdp.hpp"
#include "cerr.hpp"
#include "utils.hpp"



SDPResponse SDPDevice::read_register(uint32_t reg_addr){
    auto cmd_little_e = SDPCommand{
        .command_type = SDPCommandType::READ_REGISTER,
        .addr = reg_addr,
        .format=SDPFormat::ACCESS_32BIT,
        .data_count=4,
        .data=0,
        ._reserved=0
    };
    send_command(cmd_little_e);
    // probe point

    // get sec config
    auto response_sec_config = get_response(3, 4);
    // probe point

    // get reg value
    auto response_reg_v = get_response(4, 4);
    // probe point


    return response_reg_v;

}

// datacount is in bytes
SDPResponse SDPDevice::read_registers(uint32_t reg_addr, uint32_t data_count){
    if (data_count==0){
        throw std::invalid_argument("well, 0 datacount is nonsense whatsoever");
    }
    // we always first align datacount to 4 bytes(sizeof(uint32_t))
    data_count = align(data_count, sizeof(uint32_t));

    auto cmd_little_e = SDPCommand{
        .command_type = SDPCommandType::READ_REGISTER,
        .addr = reg_addr,
        .format=SDPFormat::ACCESS_32BIT,
        .data_count=data_count,
        .data=0,
        ._reserved=0
    };
    send_command(cmd_little_e);
    // probe point

    // get sec config
    auto response_sec_config = get_response(3, 4);
    // probe point

    // get read result
    auto response_regs = get_response(4, data_count);
    // probe point
    
    return response_regs;


}


SDPResponse SDPDevice::write_register(uint32_t reg_addr, SDPFormat format, uint32_t data)
{
    auto cmd_little_e = SDPCommand{
        .command_type = SDPCommandType::WRITE_REGISTER,
        .addr = reg_addr,
        .format=format,
        .data_count=4,
        .data=data,
        ._reserved=0
    };
    send_command(cmd_little_e);
    // probe point

    // get sec config
    auto response_sec_config = get_response(3, 4);
    // probe point

    // get wr status
    auto response_wr_status = get_response(4, 4);
    // probe point

    return response_wr_status;
}

SDPResponse SDPDevice::write_file(uint32_t dst_addr, uint8_t* filebuf, uint32_t len){
    auto cmd_little_e = SDPCommand{
        .command_type = SDPCommandType::WRITE_FILE,
        .addr = dst_addr,
        .format=SDPFormat::ACCESS_32BIT,
        .data_count=len,
        .data=0,
        ._reserved=0
    };
    send_command(cmd_little_e);
    // probe point

    //send file 
    send_data(filebuf,len);

    // get sec config
    auto response_sec_config = get_response(3, 4);
    // probe point

    auto response_wr_status = get_response(4, 4);
    // probe point

    return response_wr_status;
}


SDPResponse SDPDevice::dcd_write(uint32_t temp_dcd_addr, uint8_t* dcdbuf, uint32_t len){
    auto cmd_little_e = SDPCommand{
        .command_type = SDPCommandType::DCD_WRITE,
        .addr = temp_dcd_addr,
        .format=SDPFormat::ACCESS_32BIT,
        .data_count=len,
        .data=0,
        ._reserved=0
    };
    send_command(cmd_little_e);
    // probe point

    // send dcd
    send_data(dcdbuf,len);
    // probe point

    // get sec config
    auto response_sec_config = get_response(3, 4);
    // probe point

    // get wr status
    auto response_wr_status = get_response(4, 4);
    // probe point

    return response_wr_status;
}

SDPResponse SDPDevice::jump_address(uint32_t dst_addr){
    auto cmd_little_e = SDPCommand{
        .command_type = SDPCommandType::JUMP_ADDRESS,
        .addr = dst_addr,
        .format=SDPFormat::ACCESS_32BIT,
        .data_count=0,
        .data=0,
        ._reserved=0
    };
    send_command(cmd_little_e);

    // get sec config
    auto response_sec_config = get_response(3, 4);
    // probe point
    
    timeval tv;
    tv.tv_sec = 1; // set 1 sec timeout
    tv.tv_usec = 0;
    bool has_respond = this->hidraw_device.select_read(&tv);
    if (!has_respond){
        // success: no err response 
        return response_sec_config;
    } else {    
        fprintf(stderr, "WARNING: got error response in executing jump\n");
        auto response_er_status = get_response(4, 4);
        return response_er_status;
    }
    
}
SDPResponse SDPDevice::skip_dcd(){
    auto cmd_little_e = SDPCommand{
        .command_type = SDPCommandType::SKIP_DCD_HEADER,
        .addr = 0,
        .format=SDPFormat::ACCESS_32BIT,
        .data_count=0,
        .data=0,
        ._reserved=0
    };
    send_command(cmd_little_e);

    // get sec config
    auto response_sec_config = get_response(3, 4);
    // probe point

    // get err status
    auto response_er_status = get_response(4, 4);
    // probe point

    return response_er_status;
}





void SDPDevice::send_command(const SDPCommand& cmd){
    // adjust endianess
    SDPCommand cmd_copy{cmd};
    cmd_copy.addr = BE32(cmd_copy.addr);
    cmd_copy.data = BE32(cmd_copy.data);
    cmd_copy.data_count = BE32(cmd_copy.data_count);
    this->hidraw_device.write_obj(1, cmd_copy);
}

void SDPDevice::send_data(const uint8_t* input_data,size_t data_len){
    for (size_t byte_index =0; byte_index<data_len;byte_index+=1024){
        auto cur_sendsize = min(1024ul,data_len-byte_index);
        this->hidraw_device.write_rawdata(2, input_data+byte_index, cur_sendsize);
    }
    
}

template <typename T>
void SDPDevice::send_data(const T &obj){
    this->hidraw_device.write_obj(2, obj);
}



SDPResponse SDPDevice::get_response(uint8_t report_id,size_t response_data_len){
    if (response_data_len==sizeof(uint32_t)){
        uint32_t t;
        this->hidraw_device.read_obj_idchk(report_id, t);
        auto ret_response = SDPResponse::make_single(t);
        return ret_response;
    }
    else {
        auto ret_response = SDPResponse::make_multi(response_data_len);
        auto* response_buffer_ptr = ret_response.enlarge_buffer(response_data_len);
        for (size_t byte_index=0;byte_index<response_data_len;byte_index+=1024){
            auto cur_readsize = min(1024ul,response_data_len-byte_index);
            this->hidraw_device.read_rawdata_idchk(report_id, response_buffer_ptr+byte_index, cur_readsize);
        }
        return ret_response;
    }
}

