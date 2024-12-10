cmake_minimum_required(VERSION 3.5)

project(teahypercube LANGUAGES CXX)


set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#add_subdirectory(src)

add_executable(hypercube hypercube.cpp StatisticalTestBase.cpp)
add_executable(rewind rewind.cpp)
