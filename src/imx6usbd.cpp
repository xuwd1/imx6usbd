


// unix
#include <exception>
#include <fcntl.h>
#include <stdexcept>
#include <unistd.h>

// C 
#include <cstdint>
#include <cerrno>
#include <cstdio>
#include <cstring>
#include <iostream>

//custom
#include "cerr.hpp"
#include "sdp.hpp"
#include "imxparser.hpp"
#include "udevfind.hpp"
#include "argparse/argparse.hpp"


int main(int argc, char **argv)
{

    argparse::ArgumentParser parser("I.MX6ULL USB image downloader","imx6usbd 0.1.1",argparse::default_arguments::all,true);
    parser.add_argument("path")
        .help("path to imx image to download")
        .metavar("PATH");
    parser.add_argument("--dcdla","-d")
        .help("dcd load address, provide with prefix 0x! [default = 0x00907000]")
        .metavar("ADDR")
        .scan<'x', uint32_t>();
    parser.add_epilog("Example usage: imx6usbd --dcdla 0x00910000 /path/to/img.imx");

    try {
        parser.parse_args(argc,argv);
    }
    catch (const std::runtime_error& err) {
        std::cerr << err.what() << std::endl;
        std::cerr << parser;
        return 1;
  }

    try {  
        auto dev_path = udev_get_hidraw_devnod(0x15a2, 0x0080);
        auto image_path = parser.get<std::string>("path");
        uint32_t dcdla;
        if (parser.is_used("--dcdla")){
            dcdla = parser.get<uint32_t>("--dcdla");
        } else {
            dcdla = 0x00907000;
        }
        

        auto sdp_device = SDPDevice(dev_path);
        IMxImage img{image_path};
        auto img_parse_result = img.parse();
        
        // do critical initializations first by writing dcd
        auto response_dcd = sdp_device.dcd_write(
            dcdla, 
            img.get_file_buf()+img_parse_result.dcd_offset, 
            img_parse_result.dcd_size);


        auto response_file_wr = sdp_device.write_file(
            img_parse_result.image_load_addr,
            img.get_file_buf(),
            img.get_file_size()
        );

        auto response_skip = sdp_device.skip_dcd();


        auto response_jump = sdp_device.jump_address(
            img_parse_result.image_load_addr
        );
    } catch (std::exception& e) {
        std::cerr<<"Exception occured: "<< e.what()<<std::endl;
    }

    
    return 0;
}