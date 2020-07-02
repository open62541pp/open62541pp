enable_testing()

file(GLOB test_sources "*.cpp")
add_executable(open62541pp_tests ${test_sources})
target_link_libraries(open62541pp_tests PUBLIC open62541pp::open62541pp open62541pp_project_options)
set_target_properties(open62541pp_tests PROPERTIES OUTPUT_NAME tests)

# parse catch tests for ctest
include(CTest)
include("../lib/catch2/contrib/Catch.cmake")
catch_discover_tests(open62541pp_tests)

# run tests after build
add_custom_command(TARGET open62541pp_tests POST_BUILD
                   COMMAND ${CMAKE_CTEST_COMMAND} "--output-on-failure"
                   WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
