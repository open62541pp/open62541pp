option(UAPP_ENABLE_CLANG_TIDY "Enable testing with clang-tidy" OFF)
if(UAPP_ENABLE_CLANG_TIDY)
    message(STATUS "Static code analysis with clang-tidy enabled")

    # https://gitlab.kitware.com/cmake/cmake/-/issues/22081
    if(UAPP_ENABLE_PCH AND NOT CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        message(SEND_ERROR "clang-tidy only works with Clang when UAPP_ENABLE_PCH is enabled")
    endif()

    find_program(CLANG_TIDY_EXE NAMES clang-tidy)
    if(CLANG_TIDY_EXE)
        message(STATUS "Found clang-tidy: ${CLANG_TIDY_EXE}")
        set(CMAKE_CXX_CLANG_TIDY "${CLANG_TIDY_EXE}")
    else()
        message(SEND_ERROR "clang-tidy requested but executable not found")
    endif()
endif()
