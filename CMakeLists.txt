cmake_minimum_required(VERSION 3.16)

project(TinySpice LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(src)
add_subdirectory(sandbox)

enable_testing()
add_subdirectory(tests)

