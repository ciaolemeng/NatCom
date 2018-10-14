project(NatCom)
cmake_minimum_required(VERSION 2.8.4)


include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
CHECK_CXX_COMPILER_FLAG("-std=c++0x" COMPILER_SUPPORTS_CXX0X)

if(COMPILER_SUPPORTS_CXX11)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
elseif(COMPILER_SUPPORTS_CXX0X)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
else()
    message(STATUS "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support. Please use a different C++ compiler.")
endif()

if(${ANDROID_PHONE})
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pie -fPIE")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pie -fPIE")
elseif(${VOS})
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pie -fPIE")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pie -fPIE")
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -o3")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wl,--no-as-needed")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -lstdc++")
#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -o3")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99")
#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pthread")

#将可执行文件和库文件目录的分别设为build/bin和build/lib,同时清空编译过程的中间文件
set(CMAKE_BINARY_DIR ${PROJECT_SOURCE_DIR}/build)#set as the project directory
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)#.a/.lib
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)#.so/.dll
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)#.exe

include(InstallRequiredSystemLibraries)
include_directories(inc)
add_subdirectory(src)
