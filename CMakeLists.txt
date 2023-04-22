cmake_minimum_required(VERSION 3.12)

project(
    open62541++
    VERSION 0.2.0
    DESCRIPTION "C++ wrapper for open62541"
    HOMEPAGE_URL "https://github.com/open62541pp/open62541pp"
    LANGUAGES CXX
)

# set c++ standard explicitly, compile feature "cxx_std_17" does not set -std=c++17 compile flag
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# generate compile_commands.json
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# compiled binaries folders (same as open62541)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)  # for find_package

# interface "library" to set the c++ standard / compile-time options requested
add_library(open62541pp_project_options INTERFACE)
target_compile_features(open62541pp_project_options INTERFACE cxx_std_17)

if(MSVC)
    target_compile_options(
        open62541pp_project_options
        INTERFACE
            /permissive-
            /W4
            /w14640
    )
else()
    option(UAPP_ENABLE_COVERAGE "Enable coverage reporting for gcc/clang" OFF)
    if(UAPP_ENABLE_COVERAGE)
        message(STATUS "Coverage enabled")
        target_compile_options(open62541pp_project_options INTERFACE --coverage -O0 -g)
        target_link_libraries(open62541pp_project_options INTERFACE --coverage)
    endif()

    option(UAPP_ENABLE_CLANG_TIDY "Enable testing with clang-tidy" OFF)
    if(UAPP_ENABLE_CLANG_TIDY)
        message(STATUS "Static code analysis with clang-tidy enabled")
        find_program(CLANG_TIDY_EXE NAMES clang-tidy)
        if(CLANG_TIDY_EXE)
            message(STATUS "Found clang-tidy: ${CLANG_TIDY_EXE}")
            set(CMAKE_CXX_CLANG_TIDY "${CLANG_TIDY_EXE}")
        else()
            message(SEND_ERROR "clang-tidy requested but executable not found")
        endif()
    endif()

    target_compile_options(
        open62541pp_project_options
        INTERFACE
            -Wall
            -Wextra
            -Wshadow
            -Wnon-virtual-dtor
            -Wpedantic
    )
endif()

include(cmake/Sanitizers.cmake)

# threads
find_package(Threads REQUIRED)

# open62541
option(UAPP_INTERNAL_OPEN62541 "Use internal open62541 library" ON)
if(UAPP_INTERNAL_OPEN62541)
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

    set(OPEN62541_VERSION v1.3.5)  # set manually for CI
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
    src/Client.cpp
    src/CustomLogger.cpp
    src/Server.cpp
    src/detail/helper.cpp
    src/overloads/ostream.cpp
    src/services/Attribute.cpp
    src/services/NodeManagement.cpp
    src/services/View.cpp
    src/types/Builtin.cpp
    src/types/Composed.cpp
    src/types/DataValue.cpp
    src/types/DateTime.cpp
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

# tests
option(UAPP_BUILD_TESTS "Build unit tests" OFF)
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
        make_directory(${coverage_html_output_dir})
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
