#include "imxparser.hpp"
#include "utils.hpp"
#include <cstdint>
#include <stdexcept>

size_t locate_ivt(uint8_t* buf, size_t buflen){
    IMxIVT* ivt_mask;
    auto fcheck = [&]()->bool{
        return 
        ivt_mask->tag==0xD1 && 
        (ivt_mask->version==0X40 || ivt_mask->version==0x41) &&
        ivt_mask->reserved1==0 &&
        ivt_mask->reserved2==0;
    };
    for (size_t offset=0;offset<buflen-sizeof(IMxIVT);offset++){
        ivt_mask = reinterpret_cast<IMxIVT*>(buf+offset);
        if (fcheck()){
            return offset;
        }
    }
    return -1;
}

bool validate_dcd(IMxDCDHeader* dcd_header){
    return dcd_header->tag == 0xD2 && (dcd_header->version == 0x40 || dcd_header->version == 0x41);
}

IMxImageParseResult IMxImage::parse(){
    
    auto ivt_offset = locate_ivt(file_mmap, filesize);
    if (ivt_offset==-1){
        throw std::invalid_argument("ivt not found in parsing image");
    }
    IMxIVT* ivt = reinterpret_cast<IMxIVT*>(file_mmap+ivt_offset);
    uint16_t ivt_size = BE16(ivt->length);
    uint32_t entry_addr = ivt->entry;
    uint32_t image_load_addr = ivt->self; // load the image at ivt address!!!!

    size_t dcd_offset = ivt_offset+ ivt->dcd_ptr - ivt->self;
    IMxDCDHeader* dcd_header = reinterpret_cast<IMxDCDHeader*>(file_mmap+dcd_offset);
    if (!validate_dcd(dcd_header)){
        throw std::invalid_argument("invalid dcd encountered in parsing image");
    }
    uint16_t dcd_size = BE16(dcd_header->length);


    size_t boot_data_offset = ivt_offset + ivt->boot_data_ptr - ivt->self;
    IMxBootData* bootdata = reinterpret_cast<IMxBootData*>(file_mmap+boot_data_offset);
    
    
    
    IMxImageParseResult ret{
        .ivt_offset = ivt_offset,
        .ivt_size = ivt_size,
        .entry_addr = entry_addr,
        .dcd_offset = dcd_offset,
        .dcd_size = dcd_size,
        .boot_data_offset = boot_data_offset,
        .image_load_addr = image_load_addr,

    };
    return ret;
}