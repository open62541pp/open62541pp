cmake_minimum_required(VERSION 3.10)

enable_testing()

file(GLOB test_sources "*.cpp")
add_executable(tests ${test_sources})
target_link_libraries(tests PUBLIC open62541pp::open62541pp project_options)

# parse catch tests for ctest
include(CTest)
include("../lib/catch2/contrib/Catch.cmake")
catch_discover_tests(tests)

# run tests after build
add_custom_command(TARGET tests POST_BUILD
                   COMMAND ${CMAKE_CTEST_COMMAND} "--output-on-failure"
                   WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
