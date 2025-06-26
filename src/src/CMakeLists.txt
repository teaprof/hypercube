add_library(tearng
./app/progress/tictoc.cpp
./app/rewind.cpp
./stattests/stat_tests/chi2based/StatisticalTestBase.cpp
)

add_executable(hypercube ./app/hypercube.cpp)
target_link_libraries(hypercube tearng Boost::program_options Boost::json)

add_executable(taskgenerator ./app/taskgenerator.cpp)
target_link_libraries(taskgenerator tearng Boost::program_options Boost::json)

add_executable(rewind ./app/rewind.cpp)

add_executable(bench ./bench/bench.cpp)
target_link_libraries(bench PRIVATE tearng benchmark::benchmark_main)
target_include_directories(bench PRIVATE benchmark::include_directories)

add_executable(mybench ./bench/mybench.cpp)
target_link_libraries(mybench PRIVATE tearng)
