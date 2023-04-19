add_subdirectory(${PROJECT_SOURCE_DIR}/3rdparty/doctest doctest)

add_executable(
    open62541pp_tests
    main.cpp
    Client.cpp
    helper.cpp
    Node.cpp
    overloads.cpp
    Server.cpp
    Services.cpp
    TypeConverter.cpp
    Types.cpp
    TypeWrapper.cpp
)
target_link_libraries(
    open62541pp_tests
    PRIVATE
        doctest::doctest
        open62541pp::open62541pp
        open62541pp_project_options
)
target_include_directories(open62541pp_tests PRIVATE ../src)
set_target_properties(
    open62541pp_tests
    PROPERTIES
        OUTPUT_NAME tests
        CXX_CLANG_TIDY ""  # disable clang-tidy
)

# discover tests for ctest
include(${PROJECT_SOURCE_DIR}/3rdparty/doctest/scripts/cmake/doctest.cmake)
doctest_discover_tests(open62541pp_tests)

# run tests after build
add_custom_command(
    TARGET open62541pp_tests POST_BUILD
    COMMAND ${CMAKE_CTEST_COMMAND} "--output-on-failure"
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)
