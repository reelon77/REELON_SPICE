cmake_minimum_required(VERSION 3.16)

project(TinySpice LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 供 clangd 用；源码里有中文注释/字符串，MSVC 默认按本地代码页(936)读会截断字面量
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
if(MSVC)
    add_compile_options(/utf-8)
endif()

add_subdirectory(src)
add_subdirectory(sandbox)

enable_testing()
add_subdirectory(tests)

