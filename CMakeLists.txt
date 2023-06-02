cmake_minimum_required(VERSION 3.5)

project(teahypercube LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#find_package(Boost 1.7 REQUIRED COMPONENTS math) #math

include(FetchContent)

fetchcontent_declare(
    BoostMath
    GIT_REPOSITORY https://github.com/boostorg/math.git
    GIT_TAG boost-1.79.0)
fetchcontent_makeavailable(BoostMath)

add_executable(teahypercube src/main.cpp)
target_link_libraries(teahypercube Boost::math)
