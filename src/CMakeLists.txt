add_library(tearng
./test.cpp
./progress/tictoc.cpp
./bench.cpp
./rewind.cpp
./stat_tests/chi2based/StatisticalTestBase.cpp
)

add_executable(hypercube hypercube.cpp)
target_link_libraries(hypercube tearng Boost::program_options)

add_executable(rewind rewind.cpp)

add_executable(FooTest test.cpp)
target_link_libraries(FooTest GTest::gtest_main)
target_include_directories(FooTest PUBLIC GTEST_INCLUDE_DIRS)
gtest_discover_tests(FooTest)


add_executable(bench bench.cpp)
target_link_libraries(bench PRIVATE benchmark::benchmark_main)
target_include_directories(bench PRIVATE benchmark::include_directories)
