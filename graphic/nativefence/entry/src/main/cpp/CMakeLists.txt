# the minimum version of CMake.
cmake_minimum_required(VERSION 3.4.1)
project(nativefence)

set(NATIVERENDER_ROOT_PATH ${CMAKE_CURRENT_SOURCE_DIR})

include_directories(${NATIVERENDER_ROOT_PATH}
                    ${NATIVERENDER_ROOT_PATH}/include)

add_library(nativefence SHARED NativeFenceTest.cpp)
target_link_libraries(nativefence PUBLIC libhilog_ndk.z.so libace_napi.z.so libnative_vsync.so libnative_image.so libnative_buffer.so libnative_window.so libnative_fence.so)
