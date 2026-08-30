# 拉取 GTest（配置阶段下载一次，之后走缓存）
include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/releases/download/v1.17.0/googletest-1.17.0.tar.gz
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
FetchContent_MakeAvailable(googletest)

include(GoogleTest)

# 每个模块一个测试可执行文件
add_executable(test_matrix test_matrix.cpp)
target_link_libraries(test_matrix PRIVATE SpiceLib GTest::gtest_main)
gtest_discover_tests(test_matrix)

add_executable(test_lu test_lu.cpp)
target_link_libraries(test_lu PRIVATE SpiceLib GTest::gtest_main)
gtest_discover_tests(test_lu)

add_executable(test_mna test_mna.cpp)
target_link_libraries(test_mna PRIVATE SpiceLib GTest::gtest_main)
gtest_discover_tests(test_mna)

add_executable(test_sources test_sources.cpp)
target_link_libraries(test_sources PRIVATE SpiceLib GTest::gtest_main)
gtest_discover_tests(test_sources)

add_executable(test_diode test_diode.cpp)
target_link_libraries(test_diode PRIVATE SpiceLib GTest::gtest_main)
gtest_discover_tests(test_diode)

add_executable(test_newton test_newton.cpp)
target_link_libraries(test_newton PRIVATE SpiceLib GTest::gtest_main)
gtest_discover_tests(test_newton)

add_executable(test_parser test_parser.cpp)
target_link_libraries(test_parser PRIVATE SpiceLib GTest::gtest_main)
gtest_discover_tests(test_parser)

add_executable(test_end_to_end test_end_to_end.cpp)
target_link_libraries(test_end_to_end PRIVATE SpiceLib GTest::gtest_main)
gtest_discover_tests(test_end_to_end)

add_executable(test_capacitor test_capacitor.cpp)
target_link_libraries(test_capacitor PRIVATE SpiceLib GTest::gtest_main)
gtest_discover_tests(test_capacitor)

add_executable(test_transient test_transient.cpp)
target_link_libraries(test_transient PRIVATE SpiceLib GTest::gtest_main)
gtest_discover_tests(test_transient)

add_executable(test_inductor test_inductor.cpp)
target_link_libraries(test_inductor PRIVATE SpiceLib GTest::gtest_main)
gtest_discover_tests(test_inductor)

add_executable(test_simulate test_simulate.cpp)
target_link_libraries(test_simulate PRIVATE SpiceLib GTest::gtest_main)
gtest_discover_tests(test_simulate)

add_executable(test_result_writer test_result_writer.cpp)
target_link_libraries(test_result_writer PRIVATE SpiceLib GTest::gtest_main)
gtest_discover_tests(test_result_writer)

add_executable(test_cli test_cli.cpp)
target_link_libraries(test_cli PRIVATE TinySpiceCli GTest::gtest_main)
gtest_discover_tests(test_cli)

add_test(
    NAME CliProcessOpSmoke
    COMMAND $<TARGET_FILE:TinySpice>
            ${CMAKE_CURRENT_SOURCE_DIR}/fixtures/cli_divider.cir
)
set_tests_properties(
    CliProcessOpSmoke
    PROPERTIES PASS_REGULAR_EXPRESSION "V\\(2\\) = 8"
)

add_test(
    NAME CliProcessTransientSmoke
    COMMAND $<TARGET_FILE:TinySpice>
            ${CMAKE_CURRENT_SOURCE_DIR}/fixtures/cli_rc.cir
)
set_tests_properties(
    CliProcessTransientSmoke
    PROPERTIES PASS_REGULAR_EXPRESSION "time,V\\(in\\),V\\(out\\),I\\(v1\\)"
)

add_test(
    NAME CliProcessDcSmoke
    COMMAND $<TARGET_FILE:TinySpice>
            ${CMAKE_CURRENT_SOURCE_DIR}/fixtures/cli_dc.cir
)
set_tests_properties(
    CliProcessDcSmoke
    PROPERTIES PASS_REGULAR_EXPRESSION "sweep\\(v1\\),V\\(in\\),V\\(out\\),I\\(v1\\)"
)

add_test(
    NAME CliProcessMissingFileSmoke
    COMMAND ${CMAKE_COMMAND}
            "-DTINYSPICE_EXE=$<TARGET_FILE:TinySpice>"
            "-DINPUT_PATH=${CMAKE_CURRENT_BINARY_DIR}/definitely-missing.cir"
            -P ${CMAKE_CURRENT_SOURCE_DIR}/check_cli_failure.cmake
)

add_test(
    NAME ExampleOperatingPoint
    COMMAND $<TARGET_FILE:TinySpice>
            ${PROJECT_SOURCE_DIR}/examples/divider_op.cir
)
set_tests_properties(
    ExampleOperatingPoint
    PROPERTIES PASS_REGULAR_EXPRESSION "V\\(2\\) = 8"
)

add_test(
    NAME ExampleRcTransient
    COMMAND $<TARGET_FILE:TinySpice>
            ${PROJECT_SOURCE_DIR}/examples/rc_transient.cir
)
set_tests_properties(
    ExampleRcTransient
    PROPERTIES PASS_REGULAR_EXPRESSION "time,V\\(in\\),V\\(out\\),I\\(v1\\)"
)

add_test(
    NAME ExampleRlTransient
    COMMAND $<TARGET_FILE:TinySpice>
            ${PROJECT_SOURCE_DIR}/examples/rl_transient.cir
)
set_tests_properties(
    ExampleRlTransient
    PROPERTIES PASS_REGULAR_EXPRESSION "time,V\\(out\\),V\\(in\\),I\\(l1\\),I\\(v1\\)"
)

add_test(
    NAME ExampleDcDivider
    COMMAND $<TARGET_FILE:TinySpice>
            ${PROJECT_SOURCE_DIR}/examples/dc_divider.cir
)
set_tests_properties(
    ExampleDcDivider
    PROPERTIES PASS_REGULAR_EXPRESSION "1,1,0.5,-0.0005"
)

add_test(
    NAME ExampleDcDouble
    COMMAND $<TARGET_FILE:TinySpice>
            ${PROJECT_SOURCE_DIR}/examples/dc_double.cir
)
set_tests_properties(
    ExampleDcDouble
    PROPERTIES PASS_REGULAR_EXPRESSION "2,20,2,20,-0.002,-0.01"
)

add_test(
    NAME ExampleDcCurrentDescending
    COMMAND $<TARGET_FILE:TinySpice>
            ${PROJECT_SOURCE_DIR}/examples/dc_current_descending.cir
)
set_tests_properties(
    ExampleDcCurrentDescending
    PROPERTIES PASS_REGULAR_EXPRESSION "0,0"
)

find_package(Python3 COMPONENTS Interpreter QUIET)
if(Python3_Interpreter_FOUND)
    add_test(
        NAME PlotTransientParser
        COMMAND ${Python3_EXECUTABLE}
                ${CMAKE_CURRENT_SOURCE_DIR}/test_plot_transient.py
    )
endif()
