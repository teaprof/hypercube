cmake_minimum_required(VERSION 3.5)

project(teahypercube LANGUAGES CXX)


set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

find_package(Boost 1.72.0 COMPONENTS program_options)
include_directories(${Boost_INCLUDE_DIRS})

find_package(GTest REQUIRED)
enable_testing()

find_package(benchmark REQUIRED)

include_directories(include)

add_subdirectory(src)
add_subdirectory(tests)