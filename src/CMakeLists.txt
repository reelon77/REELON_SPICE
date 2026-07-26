add_library(SpiceLib STATIC
    Spice.cpp
    Spice.h
    core/Matrix.h
    core/Matrix.cpp
    core/LU.h
    core/LU.cpp
)

target_include_directories(SpiceLib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
