add_library(REASONING_SRC OBJECT
    reachability_analyser.cpp
    hybrid_reachability_analyser.cpp
    hybrid_augmented_paving_adapter.cpp
    hybrid_augmented_paving_orbit.cpp
    augmented_paving_orbit.cpp
    neural_symbolic.cpp
)
target_include_directories(REASONING_SRC PRIVATE "${PROJECT_SOURCE_DIR}/include/reasoning")
