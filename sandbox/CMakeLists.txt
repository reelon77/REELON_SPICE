set(Sandbox cli/main.cpp)

add_executable(TinySpice ${Sandbox})
target_link_libraries(TinySpice PRIVATE SpiceLib)