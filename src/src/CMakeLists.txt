add_library(tearng
./progress/tictoc.cpp
./bench.cpp
./rewind.cpp
./stat_tests/chi2based/StatisticalTestBase.cpp
)

add_executable(hypercube hypercube.cpp)
target_link_libraries(hypercube tearng Boost::program_options Boost::json)

add_executable(taskgenerator taskgenerator.cpp)
target_link_libraries(taskgenerator tearng Boost::program_options Boost::json)

add_executable(rewind rewind.cpp)

add_executable(bench bench.cpp)
target_link_libraries(bench PRIVATE tearng benchmark::benchmark_main)
target_include_directories(bench PRIVATE benchmark::include_directories)

add_executable(mybench mybench.cpp)
target_link_libraries(mybench PRIVATE tearng)
