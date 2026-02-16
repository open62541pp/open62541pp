cmake_minimum_required(VERSION 3.12)

project(
    open62541pp
    VERSION 0.20.0
    DESCRIPTION "C++ wrapper of open62541"
    HOMEPAGE_URL "https://github.com/open62541pp/open62541pp"
    LANGUAGES CXX
)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

# compiled binaries folders (same as open62541)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_library(open62541pp_project_options INTERFACE)
target_compile_features(open62541pp_project_options INTERFACE cxx_std_17)

option(UAPP_ENABLE_COVERAGE "Enable coverage reporting for gcc/clang" OFF)
option(UAPP_WARNINGS_AS_ERRORS "Treat warnings as errors" OFF)
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES ".*Clang")
    target_compile_options(open62541pp_project_options INTERFACE
        -Wall
        -Wextra
        -Wshadow
        -Wnon-virtual-dtor
        -Wpedantic
        $<$<BOOL:${UAPP_WARNINGS_AS_ERRORS}>:-Werror>
    )
    if(UAPP_ENABLE_COVERAGE)
        target_compile_options(open62541pp_project_options INTERFACE --coverage -O0 -g)
        target_link_libraries(open62541pp_project_options INTERFACE --coverage)
    endif()
elseif(MSVC)
    target_compile_options(open62541pp_project_options INTERFACE
        /permissive-
        /W4
        /w14640
        /wd4127   # conditional expression is constant
        /wd4661   # explicit template instantiations in cpp files
        /wd4702   # unreachable code, false positive
        /wd4996   # deprecation gmtime, localtime
        $<$<BOOL:${UAPP_WARNINGS_AS_ERRORS}>:/WX>
    )
endif()

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES ".*Clang")
    set(sanitizers "")
    foreach(san IN ITEMS address leak undefined thread)
        string(TOUPPER ${san} san_upper)
        option(UAPP_ENABLE_SANITIZER_${san_upper} "Enable ${san} sanitizer" OFF)
        if(UAPP_ENABLE_SANITIZER_${san_upper})
            list(APPEND sanitizers ${san})
        endif()
    endforeach()

    list(JOIN sanitizers "," sanitizers_joined)
    if(sanitizers_joined)
        message(STATUS "Sanitizers enabled: ${sanitizers_joined}")
        target_compile_options(open62541pp_project_options INTERFACE -fsanitize=${sanitizers_joined})
        target_link_options(open62541pp_project_options INTERFACE -fsanitize=${sanitizers_joined})
    endif()
endif()

option(UAPP_ENABLE_CLANG_TIDY "Enable static analysis with clang-tidy" OFF)
if(UAPP_ENABLE_CLANG_TIDY)
    message(STATUS "Static code analysis with clang-tidy enabled")
    find_program(CLANG_TIDY_EXE NAMES clang-tidy)
    if(CLANG_TIDY_EXE)
        message(STATUS "Found clang-tidy: ${CLANG_TIDY_EXE}")
        if(UAPP_WARNINGS_AS_ERRORS)
            set(CMAKE_CXX_CLANG_TIDY "${CLANG_TIDY_EXE};-warnings-as-errors=*")
        else()
            set(CMAKE_CXX_CLANG_TIDY "${CLANG_TIDY_EXE}")
        endif()
    else()
        message(SEND_ERROR "clang-tidy requested but executable not found")
    endif()
endif()

# threads
find_package(Threads REQUIRED)

# open62541
option(UAPP_INTERNAL_OPEN62541 "Use internal open62541 library" ON)
if(UAPP_INTERNAL_OPEN62541)
    # disable IPO if not defined, otherwise open62541 will enable it
    # IPO increases link times, especially with sanitizers enabled
    if(NOT DEFINED CMAKE_INTERPROCEDURAL_OPTIMIZATION)
        set(CMAKE_INTERPROCEDURAL_OPTIMIZATION OFF)
    endif()

    # overwrite default open62541 options
    # disable sanitizers in debug build (> v1.3)
    if (NOT UA_ENABLE_DEBUG_SANITIZER)
        set(UA_ENABLE_DEBUG_SANITIZER OFF CACHE BOOL "")
        mark_as_advanced(UA_ENABLE_DEBUG_SANITIZER)
    endif()
    # disable sanitizers in debug build (<= v1.3) WORKAROUND
    # https://github.com/open62541/open62541/blob/v1.3.5/CMakeLists.txt#L753-L764
    if (NOT UA_ENABLE_UNIT_TESTS_MEMCHECK)
        set(UA_ENABLE_UNIT_TESTS_MEMCHECK ON CACHE BOOL "")
        mark_as_advanced(UA_ENABLE_UNIT_TESTS_MEMCHECK)
    endif()

    # disable warnings as errors for open62541
    if(NOT UA_FORCE_WERROR)
        set(UA_FORCE_WERROR OFF OFF CACHE BOOL "")
    endif()

    add_subdirectory(3rdparty/open62541)  # target open62541::open62541
else()
    find_package(open62541 REQUIRED)
endif()

# enable IPO if open62541 is compiled with IPO
get_target_property(open62541_ipo open62541::open62541 INTERPROCEDURAL_OPTIMIZATION)
if(open62541_ipo)
    message(STATUS "IPO enabled")
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION ON)
else()
    message(STATUS "IPO not enabled")
endif()

# open62541pp library (static/shared depending on option BUILD_SHARED_LIBS)
include(GNUInstallDirs)
configure_file(
    include/open62541pp/config.hpp.in
    ${PROJECT_BINARY_DIR}/include_generated/open62541pp/config.hpp
)

function(deprecated_header header_old header_new)
    configure_file(
        ${PROJECT_SOURCE_DIR}/include/open62541pp/deprecated.hpp.in
        ${PROJECT_BINARY_DIR}/include_generated/open62541pp/${header_old}
    )
endfunction()
deprecated_header(nodeids.hpp ua/nodeids.hpp)
deprecated_header(types_composed.hpp ua/types.hpp)
deprecated_header(typewrapper.hpp wrapper.hpp)

add_library(
    open62541pp
    src/callback.cpp
    src/client.cpp
    src/datatype.cpp
    src/event.cpp
    src/monitoreditem.cpp
    src/node.cpp
    src/plugin/accesscontrol.cpp
    src/plugin/accesscontrol_default.cpp
    src/plugin/create_certificate.cpp
    src/plugin/log.cpp
    src/plugin/nodesetloader.cpp
    src/plugin/nodestore.cpp
    src/server.cpp
    src/services_attribute.cpp
    src/services_method.cpp
    src/services_monitoreditem.cpp
    src/services_nodemanagement.cpp
    src/services_subscription.cpp
    src/services_view.cpp
    src/session.cpp
    src/string_utils.cpp
    src/subscription.cpp
    src/types.cpp
    src/ua_types.cpp
)
add_library(open62541pp::open62541pp ALIAS open62541pp)
target_include_directories(
    open62541pp
    PUBLIC
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include_generated>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)
target_link_libraries(
    open62541pp
    PUBLIC
        open62541::open62541
    PRIVATE
        Threads::Threads
        $<BUILD_INTERFACE:open62541pp_project_options>
)

# tests
option(UAPP_BUILD_TESTS "Build unit tests" OFF)
if(UAPP_BUILD_TESTS)
    message(STATUS "Unit tests enabled")
    enable_testing()
    add_subdirectory(tests)
endif()

# examples
option(UAPP_BUILD_EXAMPLES "Build examples" OFF)
if(UAPP_BUILD_EXAMPLES)
    message(STATUS "Examples enabled")
    add_subdirectory(examples)
endif()

# documentation
option(UAPP_BUILD_DOCUMENTATION "Build documentation" OFF)
if(UAPP_BUILD_DOCUMENTATION)
    message(STATUS "Documentation enabled")
    add_subdirectory(doc)
endif()

# install targets
install(
    TARGETS open62541pp
    EXPORT open62541ppTargets
)

install(
    DIRECTORY "${PROJECT_SOURCE_DIR}/include/"
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    FILES_MATCHING
        PATTERN "*.h"
        PATTERN "*.hpp"
)

install(
    DIRECTORY "${PROJECT_BINARY_DIR}/include_generated/"
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    FILES_MATCHING
        PATTERN "*.h"
        PATTERN "*.hpp"
)

# install cmake config files
install(
    EXPORT open62541ppTargets
    NAMESPACE open62541pp::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/open62541pp
)

include(CMakePackageConfigHelpers)
configure_package_config_file(
    "${PROJECT_SOURCE_DIR}/cmake/open62541ppConfig.cmake.in"
    "${PROJECT_BINARY_DIR}/open62541ppConfig.cmake"
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/open62541pp
)

write_basic_package_version_file(
    "${PROJECT_BINARY_DIR}/open62541ppConfigVersion.cmake"
    VERSION ${CMAKE_PROJECT_VERSION}
    COMPATIBILITY SameMinorVersion
)

install(
    FILES
        "${PROJECT_BINARY_DIR}/open62541ppConfigVersion.cmake"
        "${PROJECT_BINARY_DIR}/open62541ppConfig.cmake"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/open62541pp
)
