#include <stdexcept>
#include <unistd.h>

#include <libudev.h>
#include <cstdio>
#include <string>
#include <cstring>


std::string udev_get_hidraw_devnod(uint16_t idvender,uint16_t idproduct){
    struct udev *udev;
	struct udev_enumerate *enumerate;
	struct udev_list_entry *devices, *dev_list_entry;
	struct udev_device *dev;
    udev = udev_new();
	if (!udev) {
		throw std::runtime_error("Cannot create udev");
	}
    char idvender_str[5];
    char idproduct_str[5];
    sprintf(idvender_str, "%04x",idvender);
    sprintf(idproduct_str, "%04x",idproduct);

    enumerate = udev_enumerate_new(udev);
	udev_enumerate_add_match_subsystem(enumerate, "hidraw");
	udev_enumerate_scan_devices(enumerate);
	devices = udev_enumerate_get_list_entry(enumerate);
    udev_list_entry_foreach(dev_list_entry, devices) {
		const char *syspath;
        const char *devpath;

		syspath = udev_list_entry_get_name(dev_list_entry);
		dev = udev_device_new_from_syspath(udev, syspath);
        devpath = udev_device_get_devnode(dev);
		dev = udev_device_get_parent_with_subsystem_devtype(
		       dev,
		       "usb",
		       "usb_device");
		if (!dev) {
            throw std::runtime_error("Unable to find parent usb device.");
		}
        const char* dev_idvender = udev_device_get_sysattr_value(dev,"idVendor");
        const char* dev_idproduct = udev_device_get_sysattr_value(dev, "idProduct");
        bool vender_match = strcmp(dev_idvender, idvender_str)==0;
        bool product_match = strcmp(dev_idproduct, idproduct_str)==0;
        if (vender_match && product_match){
            return std::string(devpath);
        }
	}
    throw std::runtime_error("Cannot find hidraw devnod");
}

