#!/bin/bash
cmake -DCMAKE_TOOLCHAIN_FILE=./tools/android-cmake/android.toolchain.cmake -DANDROID_NDK=/data/tools/android-ndk-r11c -DCMAKE_BUILD_TYPE=Release -DANDROID_PHONE=1 -DANDROID_ABI="arm64-v8a" -DANDROID_NATIVE_API_LEVEL="android-21" ../

#cmake -DCMAKE_TOOLCHAIN_FILE=./tools/android-cmake/android.toolchain.cmake -DANDROID_NDK=/data/tools/android-ndk-r11c -DCMAKE_BUILD_TYPE=Release -DANDROID=1 -DUSE_OPENCL=1 -DUSE_OPENCL_LIB=/home/chris/src/vpixel_core/libs/android/libOpenCL.so -DANDROID_ABI="arm64-v8a" -DANDROID_NATIVE_API_LEVEL="android-21" ../