add_test(NAME test_python_reasoning_import
    COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/test_reasoning.py")
set_property(TEST test_python_reasoning_import PROPERTY
    ENVIRONMENT "PYTHONPATH=$<TARGET_FILE_DIR:pyariadne>")
set_property(TEST test_python_reasoning_import PROPERTY LABELS "python;reasoning")
add_dependencies(tests pyariadne)

