cmake_minimum_required(VERSION 3.5)

project(teahypercube LANGUAGES CXX)


set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(GNUInstallDirs)

find_package(Boost 1.7 REQUIRED COMPONENTS program_options) #math_c99

#include(FetchContent)

#fetchcontent_declare(
#    BoostMath
#    GIT_REPOSITORY https://github.com/boostorg/math.git
#    GIT_TAG boost-1.79.0)
#fetchcontent_makeavailable(BoostMath)

#fetchcontent_declare(
#    BoostProgramOptions
#    GIT_REPOSITORY https://github.com/boostorg/program_options
#    GIT_TAG boost-1.79.0)
#fetchcontent_makeavailable(BoostProgramOptions)

#add_executable(hypercube src/hypercube/main.cpp)
#target_link_libraries(hypercube Boost::program_options) # Boost::math_c99)

#add_executable(badgenerator src/badgenerator/badgenerator.cpp src/badgenerator/main.cpp)
#target_link_libraries(badgenerator Boost::program_options)


#include(FetchContent)

#FetchContent_Declare(
#  pybind11
#  URL https://github.com/pybind/pybind11/archive/refs/tags/v2.9.2.tar.gz
#  URL_HASH SHA256=6bd528c4dbe2276635dc787b6b1f2e5316cf6b49ee3e150264e455a0d68d19c1
#)
#FetchContent_MakeAvailable(pybind11)


find_package(Python 3.10 COMPONENTS Interpreter Development REQUIRED) #pybind11 is incompatible with python3.11 [may be fixed in the future]
message("PYTHON: ${Python_Interpreter_FOUND}")
message("PYTHON: ${Python_INCLUDE_DIRS}")
find_package(pybind11)

pybind11_add_module(teahypercube MODULE src/python/teahypercube/teahypercube.cpp src/badgenerator/badgenerator.cpp)
#target_include_directories(teahypercube PUBLIC ${Python_INCLUDE_DIRS})
include_directories(${Python_INCLUDE_DIRS})
link_libraries(${Python_LIBRARIES})
#target_link_libraries(teahypercube PRIVATE pybind11::module pybind11::lto pybind11::windows_extras)
target_link_libraries(teahypercube PRIVATE pybind11::module)
#install(TARGETS teahypercube LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}")
install(TARGETS teahypercube LIBRARY DESTINATION .)

set(ignoreMe "${SKBUILD}")
