include(CTest)
set(UNIT_TESTS
    test_analyser_contracts
    test_analyser_logging
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
    if("${TEST}" STREQUAL "test_analyser_logging")
        # Run the continuous/discrete and hybrid cases in distinct processes.
        # This avoids reusing workload-mutex addresses in one TSan lock graph.
        add_test(NAME test_analyser_logging_continuous COMMAND $<TARGET_FILE:${TEST}> --concise --continuous)
        set_property(TEST test_analyser_logging_continuous PROPERTY LABELS "reasoning")
        add_test(NAME test_analyser_logging_hybrid COMMAND $<TARGET_FILE:${TEST}> --concise --hybrid)
        set_property(TEST test_analyser_logging_hybrid PROPERTY LABELS "reasoning")
    else()
        add_test(NAME ${TEST} COMMAND $<TARGET_FILE:${TEST}> --concise)
        set_property(TEST ${TEST} PROPERTY LABELS "reasoning")
    endif()
endforeach()
add_custom_target(tests)
add_dependencies(tests ${UNIT_TESTS})

