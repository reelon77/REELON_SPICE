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
