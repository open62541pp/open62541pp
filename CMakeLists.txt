cmake_minimum_required(VERSION 3.10)

find_program(CCACHE ccache)
if(CCACHE)
    set(CMAKE_CXX_COMPILER_LAUNCHER ${CCACHE})
endif()

project(open62541pp)

# interface "library" to set the c++ standard / compile-time options requested
add_library(project_options INTERFACE)
target_compile_features(project_options INTERFACE cxx_std_17)
target_include_directories(project_options INTERFACE include lib)

if(MSVC)
    target_compile_options(project_options INTERFACE /permissive- /W4 /w14640)
else()
    option(ENABLE_COVERAGE "Enable coverage reporting for gcc/clang" OFF)

    if(ENABLE_COVERAGE)
        target_compile_options(project_options INTERFACE --coverage -O0 -g)
        target_link_libraries(project_options INTERFACE --coverage)
    endif()

    target_compile_options(project_options
                           INTERFACE -Wall
                                     -Wextra
                                     -Wshadow
                                     -Wnon-virtual-dtor
                                     -Wpedantic)
endif()

# threads
find_package(Threads REQUIRED)

# open62541
add_subdirectory(lib/open62541) # target open62541::open62541

# open62541pp library
file(GLOB_RECURSE sources "src/*.cpp")
add_library(open62541pp SHARED ${sources})
add_library(open62541pp::open62541pp ALIAS open62541pp)

target_include_directories(open62541pp PUBLIC include)

target_link_libraries(open62541pp
                      PUBLIC open62541::open62541
                      PRIVATE project_options)

# tests
add_subdirectory(tests)

# examples
add_subdirectory(examples)
