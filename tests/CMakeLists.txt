include(CTest)
set(UNIT_TESTS
    test_hybrid_reachability
    test_finite_time_reachability
    test_parallel_reachability
    test_infinite_time_reachability
    test_verify_safety
    test_iterated_map_reachability
    test_augmented_paving
    test_hybrid_augmented_paving
    test_neural_symbolic
)
foreach(TEST IN LISTS UNIT_TESTS)
    add_executable(${TEST} ${TEST}.cpp)
    target_link_libraries(${TEST} PRIVATE ariadne-reasoning reasoning ariadne-testing)
    add_test(NAME ${TEST} COMMAND $<TARGET_FILE:${TEST}> --concise)
    set_property(TEST ${TEST} PROPERTY LABELS "reasoning")
endforeach()
add_custom_target(tests)
add_dependencies(tests ${UNIT_TESTS})

