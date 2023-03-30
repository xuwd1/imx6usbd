# imx6usbd: Yet another USB image downloader for I.MX6ULL



## What is imx6usbd?

`imx6usbd` is a USB serial image downloader specifically made for NXP I.MX6ULL chip. Written in C++, it aims to provide an efficient and easily understood SDP protocol USB image downloader implementation for I.MX6ULL compared to other alternatives, e.g. [imx_usb_loader](https://github.com/boundarydevices/imx_usb_loader).



## Requirements

- This tool is for Linux only, at least porting it to Windows is beyond my capability...
- A C++ compiler that supports C++17 standard, e.g. `clang-14`
- `cmake`
- `libudev`, which should be already be there with reasonably recent distros
- Linux `hidraw` driver, which should *definitely* be there and working already so long as you are not using things like WSL

## usage

building the tool:

```shell
mkdir build
cmake -B build [-G your_preferred_generator ] [-DCMAKE_BUILD_TYPE Release|RelWithDebInfo|Debug]
cmake --build build
```



using the tool:

```shell
./build/imx6usbd

path: 1 argument(s) expected. 0 provided.
Usage: I.MX6ULL USB image downloader [--help] [--version] [--dcdla ADDR] PATH

Positional arguments:
  PATH                  path to imx image to download 

Optional arguments:
  -h, --help            shows help message and exits 
  -v, --version         prints version information and exits 
  -d, --dcdla ADDR      dcd load address, provide with prefix 0x! [default = 0x00907000] 

Example usage: imx6usbd --dcdla 0x00910000 /path/to/img.imx
```

NOTE: the image MUST be a valid `.imx` image file, check **i.MX 6ULL Application Processor Reference Manual, Ch. 8.7, Program image** if you are not sure.

## Limitations

With current version of this software 0.1.1, when built directly from the source and used as a commandline tool, `imx6usbd`  could only be used for loading a valid `.imx` image to an I.MX6ULL device. 

However above, the codes in this repository gives you an convenient way to communicate with an I.MX6ULL device using SDP protocol, for example, reading and writing values to I.MX6ULL device memory via USB connection. So it should be easy to extend the code for other uses. Just take a little time to read it if you are planning to do so and the comments inside would be useful.

Support for  `json`  action script is planned for version 0.2.0
