cmake_minimum_required(VERSION 3.5)

project(teahypercube LANGUAGES CXX)


set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#add_subdirectory(src)

include(GoogleTest)
add_executable(FooTest test.cpp)
add_executable(hypercube hypercube.cpp StatisticalTestBase.cpp)
target_link_libraries(FooTest GTest::gtest_main)
add_executable(rewind rewind.cpp)
