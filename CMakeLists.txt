cmake_minimum_required(VERSION 3.30)

project(teahypercube LANGUAGES CXX)


set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

find_package(Boost 1.81.0 NO_MODULE REQUIRED COMPONENTS program_options json)
include_directories(${Boost_INCLUDE_DIRS})

find_package(GTest REQUIRED)
enable_testing()

find_package(benchmark REQUIRED)

include_directories(include)

add_subdirectory(src)
add_subdirectory(tests)
add_subdirectory(scripts)
add_subdirectory(airflow)