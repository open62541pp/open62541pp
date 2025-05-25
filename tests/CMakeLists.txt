include(FetchContent)

option(CATCH_INSTALL_DOCS "Install documentation alongside library" OFF)
FetchContent_Declare(
    Catch2
    GIT_REPOSITORY    https://github.com/catchorg/Catch2.git
    GIT_TAG           v3.8.0
    EXCLUDE_FROM_ALL
    SYSTEM
    FIND_PACKAGE_ARGS 3.3.0
)
FetchContent_MakeAvailable(Catch2)
if(TARGET Catch2)
    set_target_properties(Catch2 Catch2WithMain PROPERTIES CXX_CLANG_TIDY "")
endif()

add_executable(
    open62541pp_tests
    async.cpp
    bitmask.cpp
    callback.cpp
    client_server_common.cpp
    client_service.cpp
    client.cpp
    datatype.cpp
    event.cpp
    exception.cpp
    exceptioncatcher.cpp
    iterator.cpp
    node.cpp
    plugin_accesscontrol.cpp
    plugin_create_certificate.cpp
    plugin_log.cpp
    plugin_nodesetloader.cpp
    pluginadapter.cpp
    result.cpp
    scope.cpp
    server.cpp
    services_attribute.cpp
    services_helper.cpp
    services_method.cpp
    services_monitoreditem.cpp
    services_nodemanagement.cpp
    services_subscription.cpp
    services_view.cpp
    session.cpp
    span.cpp
    string_utils.cpp
    subscription_monitoreditem.cpp
    traits.cpp
    typeconverter.cpp
    typeregistry.cpp
    types_builtin.cpp
    types_handling.cpp
    ua_types.cpp
    wrapper.cpp
)
target_link_libraries(
    open62541pp_tests
    PRIVATE
        open62541pp::open62541pp
        open62541pp_project_options
        Catch2::Catch2WithMain
)
target_include_directories(open62541pp_tests PRIVATE ../src)
set_target_properties(
    open62541pp_tests
    PROPERTIES
        OUTPUT_NAME tests
        CXX_CLANG_TIDY ""  # disable clang-tidy
)
target_compile_definitions(
    open62541pp_tests
    PRIVATE
        UAPP_NODESET_DIR="${PROJECT_SOURCE_DIR}/3rdparty/open62541/deps/ua-nodeset"
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

include(CTest)
include(Catch)
catch_discover_tests(open62541pp_tests)
