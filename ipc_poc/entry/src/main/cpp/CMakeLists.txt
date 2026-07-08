cmake_minimum_required(VERSION 3.5.0)
project(ipc_poc)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(
    ${NATIVERENDER_ROOT_PATH}
    ${CMAKE_CURRENT_SOURCE_DIR}
)

add_library(poc SHARED
    poc.cpp
)

target_link_libraries(poc PUBLIC
    libace_napi.z.so
    libhilog_ndk.z.so
    libipc_core.z.so
)

# dlopen 用于运行时加载 samgr，绕过 NDK 编译期链接限制
target_link_libraries(poc PUBLIC dl)
