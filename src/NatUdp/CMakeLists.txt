project(NatUdp)
cmake_minimum_required(VERSION 2.8.4)

include (InstallRequiredSystemLibraries)
include(CheckCXXCompilerFlag)
aux_source_directory(. SRC)
add_executable(NatUdp ${SRC} ../../inc/NatUdp/natudp.h)
#add_library(NatUdp SHARED ${SRC} )

if (WIN32)
    target_link_libraries(NatUdp ws2_32)
endif ()