find_package(Catch2 3)
if(NOT Catch2_FOUND)
    message(STATUS "Catch2 not found. Download with FetchContent module")
    Include(FetchContent)
    FetchContent_Declare(
        Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG        v3.2.0
    )
    FetchContent_MakeAvailable(Catch2)
    set_target_properties(Catch2 Catch2WithMain PROPERTIES CXX_CLANG_TIDY "")  # disable clang-tidy
    list(APPEND CMAKE_MODULE_PATH ${Catch2_SOURCE_DIR}/contrib)
endif()

add_executable(
    open62541pp_tests
    Client.cpp
    helper.cpp
    Node.cpp
    overloads.cpp
    Server.cpp
    Services.cpp
    TypeConverter.cpp
    Types.cpp
    TypeWrapper.cpp
)
target_link_libraries(
    open62541pp_tests
    PRIVATE
        Catch2::Catch2WithMain
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

# parse catch tests for ctest
include(CTest)
include(Catch)
catch_discover_tests(open62541pp_tests)

# run tests after build
add_custom_command(
    TARGET open62541pp_tests POST_BUILD
    COMMAND ${CMAKE_CTEST_COMMAND} "--output-on-failure"
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)
