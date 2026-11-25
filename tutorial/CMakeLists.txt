cmake_minimum_required(VERSION 3.15)
project(AriadneReasoningTutorial LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}")
find_package(AriadneReasoning REQUIRED)
add_executable(tutorial_reasoning tutorial_reasoning.cpp)
target_link_libraries(tutorial_reasoning PRIVATE AriadneReasoning::ariadne-reasoning)

# The Reasoning umbrella is the only project header used by this tutorial.
# <iostream> is the sole additional C++ standard-library header.
file(STRINGS "${CMAKE_CURRENT_SOURCE_DIR}/tutorial_reasoning.cpp"
    REASONING_TUTORIAL_INCLUDES REGEX "^[ \t]*#[ \t]*include")
set(REASONING_TUTORIAL_ALLOWED_INCLUDES
    "#include <iostream>"
    "#include \"ariadne-reasoning.hpp\"")
if(NOT "${REASONING_TUTORIAL_INCLUDES}" STREQUAL "${REASONING_TUTORIAL_ALLOWED_INCLUDES}")
    message(FATAL_ERROR "Reasoning tutorial must include only <iostream> and ariadne-reasoning.hpp")
endif()
