Include(FetchContent)
FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG        v2.13.7
)
FetchContent_MakeAvailable(Catch2)
list(APPEND CMAKE_MODULE_PATH ${Catch2_SOURCE_DIR}/contrib)

file(GLOB test_sources "*.cpp")
add_executable(open62541pp_tests ${test_sources})
target_link_libraries(
    open62541pp_tests
    PRIVATE
        Catch2::Catch2
        open62541pp::open62541pp
        open62541pp_project_options
)
set_target_properties(open62541pp_tests PROPERTIES OUTPUT_NAME tests)

# parse catch tests for ctest
include(CTest)
include(Catch)
catch_discover_tests(open62541pp_tests)

# run tests after build
add_custom_command(
    TARGET open62541pp_tests POST_BUILD
    COMMAND ${CMAKE_CTEST_COMMAND} "--output-on-failure"
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)
