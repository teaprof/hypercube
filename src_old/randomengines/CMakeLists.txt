project(librng)

set(LIBRNG_SRC 
dynamic/abstractrngengine.cpp
dynamic/RNGFactory.cpp
dynamic/uniformbitswrapper.cpp
static/adapters/inttou01.cpp
)

set(LIBRNG_TEST_SRC
test/mctools_tests.cpp
auxiliary/arithmetics_test.cpp
static/adapters/intuniform_test.cpp
static/uniformrandombitgenerators/tea/lcg_test.cpp
)

include_directories(.)

add_library(rng STATIC ${LIBRNG_SRC})
target_include_directories(rng PUBLIC ${PROJECT_SOURCE_DIR})

add_executable(testrng ${LIBRNG_TEST_SRC})
find_package(GTest REQUIRED)
target_link_libraries(testrng GTest::gtest GTest::gtest_main rng)
