add_executable(unit_tests arithmetics_test.cpp circularbuffer_test.cpp bitsrepack_test.cpp)
target_link_libraries(unit_tests GTest::gtest_main)
target_include_directories(unit_tests PUBLIC GTEST_INCLUDE_DIRS)

add_executable(hypercube_tests test.cpp)
target_link_libraries(hypercube_tests GTest::gtest_main)
target_include_directories(hypercube_tests PUBLIC GTEST_INCLUDE_DIRS)

gtest_discover_tests(hypercube_tests)

