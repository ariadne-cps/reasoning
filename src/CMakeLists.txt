add_library(REASONING_SRC OBJECT
    reachability_analyser.cpp
    hybrid_reachability_analyser.cpp
    hybrid_storage_orbit.cpp
    storage_orbit.cpp
    enclosure_storage.cpp
    neural_symbolic.cpp
)
target_include_directories(REASONING_SRC PRIVATE "${PROJECT_SOURCE_DIR}/include/reasoning")
