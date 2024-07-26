add_subdirectory(${PROJECT_SOURCE_DIR}/3rdparty/doctest doctest EXCLUDE_FROM_ALL)  # don't install

add_executable(
    open62541pp_tests
    main.cpp
    AccessControl.cpp
    async.cpp
    Bitmask.cpp
    Client.cpp
    ClientService.cpp
    Config.cpp
    Crypto.cpp
    CustomDataTypes.cpp
    DataType.cpp
    ExceptionCatcher.cpp
    ErrorHandling.cpp
    Event.cpp
    Logger.cpp
    Node.cpp
    Result.cpp
    ScopeExit.cpp
    Server.cpp
    Services.cpp
    Session.cpp
    Span.cpp
    string_utils.cpp
    Subscription_MonitoredItem.cpp
    traits.cpp
    TypeConverter.cpp
    TypeRegistry.cpp
    types_handling.cpp
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
# fix LNK4096 error with MSVC
# https://learn.microsoft.com/en-us/cpp/error-messages/tool-errors/linker-tools-warning-lnk4098
if(MSVC)
    set_target_properties(
        open62541pp_tests
        PROPERTIES
            LINK_FLAGS "/NODEFAULTLIB:libcmt.lib /NODEFAULTLIB:libcmtd.lib"
    )
endif()

# discover tests for ctest
include(${PROJECT_SOURCE_DIR}/3rdparty/doctest/scripts/cmake/doctest.cmake)
doctest_discover_tests(open62541pp_tests)
