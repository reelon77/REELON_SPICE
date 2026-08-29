add_library(TinySpiceCli STATIC
    cli/run_cli.cpp
    cli/run_cli.h
)
target_link_libraries(TinySpiceCli PUBLIC SpiceLib)
target_include_directories(TinySpiceCli PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/cli)

add_executable(TinySpice cli/main.cpp)
target_link_libraries(TinySpice PRIVATE TinySpiceCli)
