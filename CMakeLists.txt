cmake_minimum_required(VERSION 3.12)

project(
    open62541++
    VERSION 0.9.0
    DESCRIPTION "C++ wrapper of open62541"
    HOMEPAGE_URL "https://github.com/open62541pp/open62541pp"
    LANGUAGES CXX
)

# set c++ standard explicitly, compile feature "cxx_std_17" does not set -std=c++17 compile flag
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

# compiled binaries folders (same as open62541)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)  # for find_package

if(${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.16.0")
    option(UAPP_ENABLE_PCH "Use precompiled headers to speed up compilation" OFF)
endif()
option(UAPP_ENABLE_COVERAGE "Enable coverage reporting for gcc/clang" OFF)

# interface "library" to set the c++ standard / compile-time options requested
add_library(open62541pp_project_options INTERFACE)
target_compile_features(open62541pp_project_options INTERFACE cxx_std_17)

include(cmake/Sanitizers.cmake)
include(cmake/StaticAnalyzers.cmake)

if(MSVC)
    target_compile_options(
        open62541pp_project_options
        INTERFACE
            /permissive-
            /W4
            /w14640
            /wd4127  # conditional expression is constant
            /wd4661  # explicit template instantiations in cpp files
            /wd4996  # deprecation gmtime, localtime
    )
else()
    target_compile_options(
        open62541pp_project_options
        INTERFACE
            -Wall
            -Wextra
            -Wshadow
            -Wnon-virtual-dtor
            -Wpedantic
    )

    if(UAPP_ENABLE_COVERAGE)
        target_compile_options(open62541pp_project_options INTERFACE --coverage -O0 -g)
        target_link_libraries(open62541pp_project_options INTERFACE --coverage)
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

    set(OPEN62541_VERSION v1.3.7)  # set manually for CI
    add_subdirectory(3rdparty/open62541)  # target open62541::open62541
else()
    set(CMAKE_FIND_PACKAGE_PREFER_CONFIG ON)  # prefer open62541's own config file
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
add_library(
    open62541pp
    src/AccessControl.cpp
    src/Client.cpp
    src/Crypto.cpp
    src/CustomAccessControl.cpp
    src/CustomDataTypes.cpp
    src/CustomLogger.cpp
    src/DataType.cpp
    src/Event.cpp
    src/Logger.cpp
    src/MonitoredItem.cpp
    src/Node.cpp
    src/Server.cpp
    src/Session.cpp
    src/Subscription.cpp
    src/detail/helper.cpp
    src/services/Attribute.cpp
    src/services/Method.cpp
    src/services/MonitoredItem.cpp
    src/services/NodeManagement.cpp
    src/services/Subscription.cpp
    src/services/View.cpp
    src/types/Builtin.cpp
    src/types/Composed.cpp
    src/types/DataValue.cpp
    src/types/DateTime.cpp
    src/types/ExtensionObject.cpp
    src/types/NodeId.cpp
    src/types/Variant.cpp
)
add_library(open62541pp::open62541pp ALIAS open62541pp)
target_include_directories(
    open62541pp
    PUBLIC
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
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

if(UAPP_ENABLE_PCH)
    message(STATUS "PCH enabled")
    target_precompile_headers(
        open62541pp
        PUBLIC
            <chrono>
            <functional>
            <iterator>
            <memory>
            <string>
            <string_view>
            <vector>
            ${PROJECT_SOURCE_DIR}/include/open62541pp/open62541.h
        PRIVATE
            ${PROJECT_SOURCE_DIR}/src/open62541_impl.h
    )
endif()

# tests
option(UAPP_BUILD_TESTS "Build unit tests" OFF)
option(UAPP_BUILD_TESTS_AUTORUN "Run unit tests after build" OFF)
if(UAPP_BUILD_TESTS)
    message(STATUS "Unit tests enabled")
    enable_testing()
    add_subdirectory(tests)
endif()

# coverage report
if(UAPP_BUILD_TESTS AND UAPP_ENABLE_COVERAGE)
    find_program(GCOVR_EXE NAMES gcovr)
    if(GCOVR_EXE)
        set(gcovr_args "-f" "include/" "-f" "src/" "${CMAKE_BINARY_DIR}")
        add_custom_target(
            open62541pp_coverage_report
            COMMAND ${GCOVR_EXE} ${gcovr_args}
            COMMENT "Generate coverage report"
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        )
        add_custom_target(
            open62541pp_coverage_report_xml
            COMMAND ${GCOVR_EXE} ${gcovr_args} "--xml" "${CMAKE_BINARY_DIR}/coverage.xml"
            COMMENT "Generate coverage report (xml/cobertura)"
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        )
        set(coverage_html_output_dir "${CMAKE_BINARY_DIR}/coverage_html")
        file(MAKE_DIRECTORY ${coverage_html_output_dir})
        add_custom_target(
            open62541pp_coverage_report_html
            COMMAND ${GCOVR_EXE} ${gcovr_args} "--html-details" "${coverage_html_output_dir}/index.html"
            COMMENT "Generate coverage report (html)"
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        )
    endif()
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
    "${CMAKE_BINARY_DIR}/open62541ppConfig.cmake"
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/open62541pp
)

write_basic_package_version_file(
    "${CMAKE_BINARY_DIR}/open62541ppConfigVersion.cmake"
    VERSION ${CMAKE_PROJECT_VERSION}
    COMPATIBILITY SameMinorVersion
)

install(
    FILES
        "${CMAKE_BINARY_DIR}/open62541ppConfigVersion.cmake"
        "${CMAKE_BINARY_DIR}/open62541ppConfig.cmake"
        "${PROJECT_SOURCE_DIR}/cmake/Findopen62541.cmake"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/open62541pp
)
