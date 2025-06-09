cmake_minimum_required(VERSION 3.15)

# tests using doctest header
add_executable(rendripper_tests
    test_main.cpp
    ${CMAKE_SOURCE_DIR}/src/utils/IO_utility.cpp
    ${CMAKE_SOURCE_DIR}/src/gcode/GCodeParser.cpp
)

# Include project headers
target_include_directories(rendripper_tests PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/utils
    ${CMAKE_SOURCE_DIR}/src/gcode
    ${CMAKE_SOURCE_DIR}/external/doctest
)

# Only link libs needed for tested components.
# For now we only need std and such - not GL or external libs.

add_test(NAME rendripper_tests COMMAND rendripper_tests)
