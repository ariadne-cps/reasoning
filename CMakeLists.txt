cmake_minimum_required(VERSION 3.15)
project(Reasoning VERSION 1.0)

option(WARNINGS_AS_ERRORS "Treat compiler warnings as errors" OFF)
option(BUILD_PYTHON_BINDINGS "Build Python bindings" ON)

if(NOT TARGET ariadne-build)
    find_package(Git REQUIRED)
    execute_process(COMMAND "${GIT_EXECUTABLE}" -C "${CMAKE_CURRENT_SOURCE_DIR}"
                    submodule update --init -- ss/bui RESULT_VARIABLE _result)
    if(_result)
        message(FATAL_ERROR "Cannot initialize Build submodule")
    endif()
    include("${CMAKE_CURRENT_SOURCE_DIR}/ss/bui/cmake/ProjectOptions.cmake")
endif()

if(EXISTS "${PROJECT_SOURCE_DIR}/.git")
    require_same_dependency_commit(Build CHAINS
        "build" "evolution|build"
        "evolution|system|build" "evolution|integration|build"
        "evolution|equation-solving|build"
        "smt|build" "sat|build")
    require_same_dependency_commit(Symbolic CHAINS
        "evolution|system|hybrid|labelled|expression-set|symbolic" "smt|symbolic")
    require_same_dependency_commit(EquationSolving CHAINS
        "evolution|equation-solving" "smt|equation-solving")
    require_same_dependency_commit(Cdcl CHAINS "smt|cdcl" "sat|cdcl")
    require_same_dependency_commit(Bdd CHAINS
        "sat|bdd" "evolution|system|hybrid|labelled|drawing|paving|bdd")
endif()

setup_project()
include_directories("${PROJECT_SOURCE_DIR}/include")
add_subdirectory(ss)

add_library(reasoning INTERFACE)
target_include_directories(reasoning INTERFACE
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include/ariadne-reasoning>)
target_link_libraries(reasoning INTERFACE evolution smt sat)

add_subdirectory(src)
target_link_libraries(REASONING_SRC PRIVATE reasoning)

ariadne_register_public_headers(
    TARGET reasoning
    DIRECTORIES "${PROJECT_SOURCE_DIR}/include/reasoning"
    FILES "${PROJECT_SOURCE_DIR}/include/ariadne-reasoning.hpp")

setup_project_library(
    TARGET ariadne-reasoning
    OBJECTS REASONING_SRC
    LINK_LIBRARIES reasoning)

setup_standalone_project_tests(
    TARGET ariadne-reasoning
    TEST_DIRECTORY tests
    TEST_TARGET tests
    SOURCES "${PROJECT_SOURCE_DIR}/include" "${PROJECT_SOURCE_DIR}/src")

if(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
    ariadne_install_dependency_bundle(
        TARGET reasoning DESTINATION include/ariadne-reasoning)
endif()

if(BUILD_PYTHON_BINDINGS)
    add_subdirectory(python)
endif()
