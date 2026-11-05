include_directories(${Python3_INCLUDE_DIRS})
get_target_property(PYTHON_COMMON_SOURCE_DIR pyariadne-python-common SOURCE_DIR)
add_library(pyariadne-reasoning-submodule-obj OBJECT reasoning_submodule.cpp)
target_include_directories(pyariadne-reasoning-submodule-obj PRIVATE
    "${PYTHON_COMMON_SOURCE_DIR}/include"
    $<TARGET_PROPERTY:pyariadne-evolution,INTERFACE_INCLUDE_DIRECTORIES>)
target_link_libraries(pyariadne-reasoning-submodule-obj PRIVATE
    reasoning pyariadne-evolution pyariadne-python-common pybind11::module)
set_target_properties(pyariadne-reasoning-submodule-obj PROPERTIES POSITION_INDEPENDENT_CODE ON)
add_library(pyariadne-reasoning INTERFACE)
target_sources(pyariadne-reasoning INTERFACE $<TARGET_OBJECTS:pyariadne-reasoning-submodule-obj>)
target_link_libraries(pyariadne-reasoning INTERFACE pyariadne-evolution pyariadne-smt pyariadne-sat)
ariadne_add_python_module(NAME reasoning MODULE_IMPORT_NAME pyariadne_reasoning MODULE_SOURCE reasoning_module.cpp)
