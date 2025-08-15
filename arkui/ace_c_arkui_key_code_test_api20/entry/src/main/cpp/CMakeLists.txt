# the minimum version of CMake.
cmake_minimum_required(VERSION 3.5.0)
project(keyCodeNdk)

set(NATIVERENDER_ROOT_PATH ${CMAKE_CURRENT_SOURCE_DIR})

if(DEFINED PACKAGE_FIND_FILE)
    include(${PACKAGE_FIND_FILE})
endif()

include_directories(${NATIVERENDER_ROOT_PATH}
                        ${NATIVERENDER_ROOT_PATH}/include)

add_library(keycodendk SHARED nativekeycode/KeyCodeTest.cpp)

add_library(nativexcomponent SHARED
    component/NativeXComponentTest.cpp
    component/ComponentTest.cpp
    component/XComponentTest.cpp
    touch/TouchPointTest.cpp
    mouse/XComponentMouseTest.cpp
    node/NodeAttributeTest.cpp
)

find_library(
    # Sets the name of the path variable.
    hilog-lib
    # Specifies the name of the NDK library that
    # you want CMake to locate.
    hilog_ndk.z
)

find_library(
    # Sets the name of the path variable.
    libace-lib
    # Specifies the name of the NDK library that
    # you want CMake to locate.
    ace_ndk.z
)

find_library(
    # Sets the name of the path variable.
    libnapi-lib
    # Specifies the name of the NDK library that
    # you want CMake to locate.
    ace_napi.z
)

find_library(
    # Sets the name of the path variable.
    libuv-lib
    # Specifies the name of the NDK library that
    # you want CMake to locate.
    uv
)

target_link_libraries(nativexcomponent PUBLIC
    ${hilog-lib} ${libace-lib} ${libnapi-lib} ${libuv-lib} libudmf.so libace_napi.z.so libability_base_want.so)

target_link_libraries(keycodendk PUBLIC libace_napi.z.so libhilog_ndk.z.so libohinput.so)

add_library(accessibilityndk SHARED accessibilitytest/AccessibilityTest.cpp
                                    accessibilitytest/Accessibility1Test.cpp
                                    accessibilitytest/Accessibility2Test.cpp
                                    accessibilitytest/Accessibility3Test.cpp
                                    accessibilitytest/Accessibility4Test.cpp
                                    accessibilitytest/Accessibility5Test.cpp
                                    accessibilitytest/Accessibility6Test.cpp
)
target_link_libraries(accessibilityndk PUBLIC libace_napi.z.so libhilog_ndk.z.so libos_account_ndk.so libohinput.so
                            libace_ndk.z.so)

add_library(animatendk SHARED animateoptiontest/AnimatorOptionTest.cpp
                              animateoptiontest/KeyframeAnimateOptionTest.cpp
                              animateoptiontest/AnimatorOption1Test.cpp
                              animateoptiontest/AnimatorOption2Test.cpp)
target_link_libraries(animatendk PUBLIC libace_napi.z.so libhilog_ndk.z.so libos_account_ndk.so libohinput.so
                            libace_ndk.z.so)
