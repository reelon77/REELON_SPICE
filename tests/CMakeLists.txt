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
