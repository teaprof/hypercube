cmake_minimum_required(VERSION 3.5)

project(teahypercube LANGUAGES CXX)


set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Boost 1.72.0 COMPONENTS program_options)
include_directories(${Boost_INCLUDE_DIRS})

#add_subdirectory(src)

add_executable(hypercube hypercube.cpp StatisticalTestBase.cpp progress/tictoc.cpp)
target_link_libraries(hypercube Boost::program_options)
add_executable(rewind rewind.cpp)

find_package(GTest REQUIRED)
enable_testing()


add_executable(FooTest test.cpp)
target_link_libraries(FooTest GTest::gtest_main)
target_include_directories(FooTest PUBLIC GTEST_INCLUDE_DIRS)
gtest_discover_tests(FooTest)

find_package(benchmark REQUIRED)

add_executable(bench bench.cpp)
target_link_libraries(bench PRIVATE benchmark::benchmark_main)
target_include_directories(bench PRIVATE benchmark::include_directories)