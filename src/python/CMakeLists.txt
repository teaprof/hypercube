cmake_minimum_required(VERSION 3.20)
find_package(SWIG COMPONENTS python)
include(UseSWIG)

find_package(Python COMPONENTS Development)

#set(SWIG_USE_TARGET_INCLUDE_DIRECTORIES TRUE)
message("Python_INCLUDE_DIRS: ${Python_INCLUDE_DIRS}")

set_property(SOURCE pyhypercube.i PROPERTY CPLUSPLUS ON)
swig_add_library(pyhypercube LANGUAGE python SOURCES pyhypercube.i)
target_include_directories(pyhypercube PRIVATE ${Python_INCLUDE_DIRS} ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(pyhypercube PRIVATE /usr/lib/python3/dist-packages/numpy/core/include/)

