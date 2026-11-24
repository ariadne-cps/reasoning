add_library(REASONING_SRC OBJECT
    reachability_analyser.cpp
    hybrid_reachability_analyser.cpp
    hybrid_augmented_paving_adapter.cpp
    hybrid_paving.cpp
    hybrid_paving_value_operations.cpp
    hybrid_augmented_paving_orbit.cpp
    augmented_paving_orbit.cpp
    enclosure_augmented_paving.cpp
    neural_symbolic.cpp
    parallel_reachability.cpp
    time_partition.cpp
)
target_include_directories(REASONING_SRC PRIVATE "${PROJECT_SOURCE_DIR}/include/reasoning")
