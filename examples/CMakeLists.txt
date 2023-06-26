cmake_minimum_required(VERSION 3.12)

project(open62541pp_examples)

if(NOT TARGET open62541pp)
    # stand-alone build
    find_package(open62541pp REQUIRED)
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/examples)

set(examples_sources
    client_browse.cpp
    client_connect.cpp
    client_find_servers.cpp
    client_method.cpp
    client_minimal.cpp
    client_subscription.cpp
    server.cpp
    server_instantiation.cpp
    server_method.cpp
    server_minimal.cpp
)

foreach(source ${examples_sources})
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
endforeach(source ${sandbox_sources})
