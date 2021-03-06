cmake_minimum_required(VERSION 3.10)

find_program(CCACHE ccache)
if(CCACHE)
    set(CMAKE_CXX_COMPILER_LAUNCHER ${CCACHE})
endif()

project(open62541pp)

# compiled binaries folders (same as open62541)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# interface "library" to set the c++ standard / compile-time options requested
add_library(open62541pp_project_options INTERFACE)
target_compile_features(open62541pp_project_options INTERFACE cxx_std_17)
target_include_directories(open62541pp_project_options INTERFACE include lib)

if(MSVC)
    target_compile_options(open62541pp_project_options INTERFACE /permissive- /W4 /w14640)
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
        find_program(CLANGTIDY clang-tidy)
        if(CLANGTIDY)
            message(STATUS "Found clang-tidy: ${CLANGTIDY}")
            set(CMAKE_CXX_CLANG_TIDY "${CLANGTIDY}")
        else()
            message(SEND_ERROR "clang-tidy requested but executable not found")
        endif()
    endif()

    target_compile_options(open62541pp_project_options
                           INTERFACE -Wall
                                     -Wextra
                                     -Wshadow
                                     -Wnon-virtual-dtor
                                     -Wpedantic)
endif()

# options
option(BUILD_SHARED_LIBS "Enable building of shared libraries (dll/so)" OFF)

# threads
find_package(Threads REQUIRED)

# open62541
set(UA_MULTITHREADING 100 CACHE STRING "") # enable thread-safe API (100-199)
add_subdirectory(lib/open62541) # target open62541::open62541

# open62541pp library
file(GLOB_RECURSE sources "src/*.cpp")
add_library(open62541pp ${sources}) # static / shared depending on option BUILD_SHARED_LIBS
add_library(open62541pp::open62541pp ALIAS open62541pp)

target_include_directories(open62541pp PUBLIC include)

target_link_libraries(open62541pp
                      PUBLIC open62541::open62541
                      PRIVATE Threads::Threads open62541pp_project_options)

# tests
option(UAPP_BUILD_TESTS "Build unit tests" OFF)
if(UAPP_BUILD_TESTS)
    message(STATUS "Unit tests enabled")
    add_subdirectory(tests)
endif()

# examples
option(UAPP_BUILD_EXAMPLES "Build examples" OFF)
if(UAPP_BUILD_EXAMPLES)
    message(STATUS "Examples enabled")
    add_subdirectory(examples)
endif()

# documentation
add_subdirectory(doc)