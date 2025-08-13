# the minimum version of CMake.
cmake_minimum_required(VERSION 3.5.0)
project(keyCodeNdk)

set(NATIVERENDER_ROOT_PATH ${CMAKE_CURRENT_SOURCE_DIR})

if(DEFINED PACKAGE_FIND_FILE)
    include(${PACKAGE_FIND_FILE})
endif()

include_directories(${NATIVERENDER_ROOT_PATH}
                        ${NATIVERENDER_ROOT_PATH}/include)

add_library(keycodendk SHARED
    nativekeycode/KeyCodeTest.cpp
    common/CommonTest.cpp)

add_library(nativexcomponent SHARED
    component/NativeXComponentTest.cpp
    component/ComponentTest.cpp
    component/XComponentTest.cpp
    touch/TouchPointTest.cpp
    mouse/XComponentMouseTest.cpp
    node/NodeAttributeTest.cpp
    common/CommonTest.cpp
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
    ${hilog-lib} ${libace-lib} ${libnapi-lib} ${libuv-lib} libudmf.so libace_napi.z.so)

target_link_libraries(keycodendk PUBLIC libace_napi.z.so libhilog_ndk.z.so libohinput.so)

add_library(arkuikeycodendk SHARED arkuikeycode/ArkUIKeyCodeTest.cpp)
target_link_libraries(arkuikeycodendk PUBLIC libace_napi.z.so libhilog_ndk.z.so libos_account_ndk.so libohinput.so)

add_library(noderenderndk SHARED NapiRenderInitTest.cpp
    focuscontrol/FocusControlTest.cpp
    nodeevent/NodeEventTest.cpp
    manager/PluginManagerTest.cpp
    keyevent/KeyEventTest.cpp
    nodeadapter/NodeAdapterTest.cpp
    nodecontent/NodeContentTest.cpp
    snapshot/SnapShotTest.cpp
    scope/RunTaskInScopeTest.cpp
    nodenapi/NodeNapiTest.cpp
    nodenapi/NodeNapi1Test.cpp
    nodenapi/NodeNapi2Test.cpp
    nodenapi/NodeNapi3Test.cpp
    nodenapi/NodeNapi4Test.cpp
    nodenapi/NavigationTest.cpp
    nodenapi/NavDestinationTest.cpp
    nodenapi/GetRouterTest.cpp
    nodeutilstest/NodeUtils1Test.cpp
    nodeutilstest/NodeUtilsTest.cpp
    nodeutilstest/NodeUtils2Test.cpp
)

target_link_libraries(noderenderndk PUBLIC libace_napi.z.so libhilog_ndk.z.so libos_account_ndk.so libohinput.so
libace_ndk.z.so libpixelmap.so)