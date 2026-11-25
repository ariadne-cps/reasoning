cmake_minimum_required(VERSION 3.15)
project(AriadneReasoningTutorial LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}")
find_package(AriadneReasoning REQUIRED)
add_executable(tutorial_reasoning tutorial_reasoning.cpp)
target_link_libraries(tutorial_reasoning PRIVATE AriadneReasoning::ariadne-reasoning)

# The installed umbrella is the only include used by the C++ tutorial.
file(STRINGS "${CMAKE_CURRENT_SOURCE_DIR}/tutorial_reasoning.cpp"
    REASONING_TUTORIAL_INCLUDES REGEX "^[ \t]*#[ \t]*include")
if(NOT "${REASONING_TUTORIAL_INCLUDES}" STREQUAL "#include \"ariadne-reasoning.hpp\"")
    message(FATAL_ERROR "Reasoning tutorial must include only ariadne-reasoning.hpp")
endif()
