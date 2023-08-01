cmake_minimum_required(VERSION 3.12)

project(open62541pp_examples)

if(NOT TARGET open62541pp)
    # stand-alone build
    find_package(open62541pp REQUIRED)
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/examples)

function(add_example source)
    # extract filename
    get_filename_component(name ${source} NAME_WE)
    # use open62541pp_example_ prefix for target to prevent naming collisions
    set(target_name "open62541pp_examples_${name}")
    add_executable(${target_name} ${source})
    target_link_libraries(
        ${target_name}
        PRIVATE
            open62541pp::open62541pp
            $<TARGET_NAME_IF_EXISTS:open62541pp_project_options>  # not available in stand-alone
    )
    set_target_properties(
        ${target_name}
        PROPERTIES
            OUTPUT_NAME ${name}
    )
    # disable clang-tidy check to avoid try/except blocks in examples
    if(CMAKE_CXX_CLANG_TIDY)
        set(checks "-bugprone-exception-escape")  # allow uncaught exceptions in main
        set_target_properties(
            ${target_name}
            PROPERTIES
                CXX_CLANG_TIDY "${CMAKE_CXX_CLANG_TIDY};-checks=${checks}"
        )
    endif()
    if(UAPP_ENABLE_PCH)
        target_precompile_headers(
            ${target_name}
            REUSE_FROM
                open62541pp::open62541pp
        )
    endif()
endfunction()

add_example(client_browse.cpp)
add_example(client_connect.cpp)
add_example(client_find_servers.cpp)
add_example(client_minimal.cpp)
add_example(server.cpp)
add_example(server_instantiation.cpp)
add_example(server_minimal.cpp)
add_example(server_valuecallback.cpp)
add_example(server_datasource.cpp)

if(UA_ENABLE_METHODCALLS)
    add_example(client_method.cpp)
    add_example(server_method.cpp)
endif()

if(UA_ENABLE_SUBSCRIPTIONS)
    add_example(client_subscription.cpp)
endif()
