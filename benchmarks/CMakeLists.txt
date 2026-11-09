# Registered by Benchmarking in standalone Reasoning builds only.
add_executable(benchmark_infinite_time_reachability
    benchmark_infinite_time_reachability.cpp)
target_link_libraries(benchmark_infinite_time_reachability PRIVATE
    ariadne-reasoning reasoning ariadne-benchmarking)
add_dependencies(benchmarks benchmark_infinite_time_reachability)
