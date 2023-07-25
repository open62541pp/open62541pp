add_subdirectory(${PROJECT_SOURCE_DIR}/3rdparty/doctest doctest EXCLUDE_FROM_ALL)  # don't install

add_executable(
    open62541pp_tests
    main.cpp
    AccessControl.cpp
    Client.cpp
    Crypto.cpp
    helper.cpp
    Logger.cpp
    Node.cpp
    overloads.cpp
    Server.cpp
    Services.cpp
    Subscription_MonitoredItem.cpp
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
if(UAPP_ENABLE_PCH)
    target_precompile_headers(
        open62541pp_tests
        REUSE_FROM
            open62541pp
    )
endif()

# discover tests for ctest
include(${PROJECT_SOURCE_DIR}/3rdparty/doctest/scripts/cmake/doctest.cmake)
doctest_discover_tests(open62541pp_tests)

if(UAPP_BUILD_TESTS_AUTORUN)
    add_custom_command(
        TARGET open62541pp_tests POST_BUILD
        COMMAND ${CMAKE_CTEST_COMMAND} "--output-on-failure"
        COMMENT "Run ctest"
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    )
endif()
