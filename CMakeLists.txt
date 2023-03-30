cmake_minimum_required(VERSION 3.21)
project(imx6usbd VERSION 0.1.1.0 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

if (NOT CMAKE_BUILD_TYPE)
    message(WARNING "No build type specified, using RelWithDebInfo as default")
    set(CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING "build type" FORCE)
else()
    message(STATUS "Selected build type ${CMAKE_BUILD_TYPE}")
endif()

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR})
find_package(udev)
if (NOT UDEV_FOUND)
    message(FATAL_ERROR "libudev not found!")
endif()



## LIB
set(L_IMX6USBD_SRC
    src/sdp.cpp
    src/imxparser.cpp
    src/udevfind.cpp
)
add_library(imx6usbd_rt SHARED ${L_IMX6USBD_SRC})
target_link_libraries(imx6usbd_rt PUBLIC ${UDEV_LIBRARIES})
target_include_directories(imx6usbd_rt PUBLIC include)
target_include_directories(imx6usbd_rt PUBLIC ${UDEV_INCLUDE_DIRS})

### main
set(IMX6USBD_SRC
    src/imx6usbd.cpp
)
add_executable(imx6usbd ${IMX6USBD_SRC})
target_link_libraries(imx6usbd PRIVATE imx6usbd_rt)
target_include_directories(imx6usbd PRIVATE argparse/include)


