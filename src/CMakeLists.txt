add_library(REASONING_SRC OBJECT
    reachability_analyser.cpp
    hybrid_reachability_analyser.cpp
    hybrid_augmented_paving.cpp
    hybrid_augmented_paving_adapter.cpp
    hybrid_augmented_paving_orbit.cpp
    augmented_paving_orbit.cpp
    enclosure_augmented_paving.cpp
    labelled_augmented_paving.cpp
    augmented_paving_draw.cpp
    neural_symbolic.cpp
)
target_include_directories(REASONING_SRC PRIVATE "${PROJECT_SOURCE_DIR}/include/reasoning")
