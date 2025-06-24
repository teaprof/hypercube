cmake_minimum_required(VERSION 3.20)
find_package(SWIG COMPONENTS python octave)
message("SWIG found: ${SWIG_EXECUTABLE}")
message("SWIG python found: ${SWIG_python_FOUND}")
include(UseSWIG)

find_package(Python COMPONENTS Development)
include_directories(Python_INCLUDE_DIRS)
include_directories(/usr/lib/python3/dist-packages/numpy/core/include/)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

#list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}")
#find_package(Octave)
#include_directories(${OCTAVE_INCLUDE_DIRS})
#message("OCTAVE_INCLUDE_DIRS ${OCTAVE_INCLUDE_DIRS}")


set(SWIG_USE_TARGET_INCLUDE_DIRECTORIES TRUE)
message("Python_INCLUDE_DIRS: ${Python_INCLUDE_DIRS}")

set_property(SOURCE hypercube.i PROPERTY USE_TARGET_INCLUDE_DIRECTORIES ON)
set_property(SOURCE hypercube.i PROPERTY INCLUDE_DIRECTORIES /usr/lib/python3/dist-packages/numpy/core/include/)
set_property(SOURCE hypercube.i PROPERTY CPLUSPLUS ON)
swig_add_library(pyhypercube LANGUAGE python SOURCES hypercube.i)
target_include_directories(pyhypercube PRIVATE ${Python_INCLUDE_DIRS} ${PROJECT_SOURCE_DIR})

#swig_add_library(ldpco LANGUAGE octave SOURCES hypercube.i ../src/AbstractDecoder.cpp ../src/BruteForceDecoder.cpp ../src/matrix.cpp ../src/LDPCSystematicSoft.cpp)
#target_include_directories(ldpco PRIVATE ${PROJECT_SOURCE_DIR})

