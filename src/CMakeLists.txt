add_library(SpiceLib STATIC
    Spice.cpp
    Spice.h
    core/Matrix.h
    core/Matrix.cpp
    core/LU.h
    core/LU.cpp
    mna/MnaSystem.h
    mna/MnaSystem.cpp
    devices/Device.h
    devices/Resistor.h
    devices/Resistor.cpp
    devices/CurrentSource.h
    devices/CurrentSource.cpp
    devices/VoltageSource.h
    devices/VoltageSource.cpp
)

target_include_directories(SpiceLib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
