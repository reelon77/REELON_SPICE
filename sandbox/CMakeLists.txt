add_library(TinySpiceCli STATIC
    cli/run_cli.cpp
    cli/run_cli.h
    cli/matlab_plot.cpp
    cli/matlab_plot.h
)
target_link_libraries(TinySpiceCli PUBLIC SpiceLib)
target_include_directories(TinySpiceCli PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/cli)
target_compile_definitions(
    TinySpiceCli
    PRIVATE TINYSPICE_MATLAB_SCRIPT_DIR="${PROJECT_SOURCE_DIR}/scripts"
)

add_executable(TinySpice cli/main.cpp)
target_link_libraries(TinySpice PRIVATE TinySpiceCli)
