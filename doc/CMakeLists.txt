find_package(Doxygen)
if(DOXYGEN_FOUND)
    message(STATUS "Found doxygen: ${DOXYGEN_EXECUTABLE}")
endif()

find_program(SPHINX_BUILD
             NAMES sphinx-build
             DOC "Sphinx documentation generator")
if(SPHINX_BUILD)
    message(STATUS "Found sphinx-build: ${SPHINX_EXECUTABLE}")
endif()

find_package(PythonInterp)

# DOXYGEN
set(DOXYGEN_INPUT_DIR  ${PROJECT_SOURCE_DIR}/include/open62541pp)
set(DOXYGEN_OUTPUT_DIR ${PROJECT_BINARY_DIR}/doxygen)
set(DOXYGEN_INDEX_FILE ${DOXYGEN_OUTPUT_DIR}/xml/index.xml)
set(DOXYFILE_IN        ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in)
set(DOXYFILE_OUT       ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)

make_directory(${DOXYGEN_OUTPUT_DIR})
configure_file(${DOXYFILE_IN} ${DOXYFILE_OUT} @ONLY)

# find all public headers (as dependency for doxygen build)
get_target_property(OPEN62541PP_INCLUDE_DIR open62541pp INTERFACE_INCLUDE_DIRECTORIES)
file(GLOB_RECURSE OPEN62541PP_PUBLIC_HEADER ${OPEN62541PP_INCLUDE_DIR}/*.h)

add_custom_command(
    OUTPUT ${DOXYGEN_INDEX_FILE}
    DEPENDS ${OPEN62541PP_PUBLIC_HEADER}
    COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYFILE_OUT}
    MAIN_DEPENDENCY ${DOXYFILE_OUT} ${DOXYFILE_IN}
    COMMENT "Generating doxygen documentation..."
    VERBATIM
)

# SPHINX
set(DOC_HTML_DIR ${PROJECT_BINARY_DIR}/doc_html)
make_directory(${DOC_HTML_DIR})

set(SPHINX_CONF_FILE ${CMAKE_CURRENT_SOURCE_DIR}/conf.py)

add_custom_target(
    doc-html
    COMMAND
        ${SPHINX_BUILD} -b html
        # tell breathe where to find the Doxygen output
        -Dbreathe_projects.open62541pp=${DOXYGEN_OUTPUT_DIR}/xml
        ${CMAKE_CURRENT_SOURCE_DIR} ${DOC_HTML_DIR}
    DEPENDS ${SPHINX_BUILD} ${SPHINX_CONF_FILE} ${DOXYGEN_INDEX_FILE}
    COMMENT "Building HTML documentation with Sphinx"
)

# DOXYGEN OPEN62541 (for development purpose - easy to browse through the open62541 API)
if(UAPP_INTERNAL_OPEN62541)
    set(OPEN62541_DOXYGEN_INPUT_DIR  ${CMAKE_CURRENT_BINARY_DIR}/open62541-api)  # public header files are copied and processed here
    set(OPEN62541_DOXYGEN_OUTPUT_DIR ${PROJECT_BINARY_DIR}/doxygen_open62541)
    set(OPEN62541_DOXYGEN_INDEX_FILE ${DOXYGEN_OUTPUT_DIR}/html/index.html)
    set(OPEN62541_DOXYFILE_IN        ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile_open62541.in)
    set(OPEN62541_DOXYFILE_OUT       ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile_open62541)

    make_directory(${OPEN62541_DOXYGEN_OUTPUT_DIR})
    configure_file(${OPEN62541_DOXYFILE_IN} ${OPEN62541_DOXYFILE_OUT} @ONLY)

    get_target_property(OPEN62541_SOURCE_DIR  open62541::open62541 SOURCE_DIR)
    get_target_property(OPEN62541_BINARY_DIR  open62541::open62541 BINARY_DIR)
    get_target_property(OPEN62541_INCLUDE_DIR open62541::open62541 INTERFACE_INCLUDE_DIRECTORIES)
    file(GLOB_RECURSE OPEN62541_PUBLIC_HEADER ${OPEN62541_INCLUDE_DIR}/*.h)

    # copy all public interface files to a new folder (to strip rst comments and keywords)
    list(APPEND OPEN62541_COPY_LIST "")
    foreach(dir ${OPEN62541_INCLUDE_DIR})
        string(REGEX REPLACE "\\$<[a-zA-Z_]+:(.+)>" "\\1" dir_stripped ${dir}) # strip generator expression

        string(REPLACE "${OPEN62541_SOURCE_DIR}" ${OPEN62541_DOXYGEN_INPUT_DIR} dir_target "${dir_stripped}")
        string(REPLACE "${OPEN62541_BINARY_DIR}" ${OPEN62541_DOXYGEN_INPUT_DIR} dir_target "${dir_target}")

        add_custom_command(
            OUTPUT ${dir_target}
            COMMAND ${CMAKE_COMMAND} -E copy_directory ${dir_stripped} ${dir_target}
            COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/prepare_open62541_header.py ${dir_target}
            COMMENT "Copy ${dir_stripped} -> ${dir_target} and prepare for Doxygen..."
        )

        list(APPEND OPEN62541_COPY_LIST ${dir_target})
    endforeach()

    add_custom_command(
        OUTPUT ${OPEN62541_DOXYGEN_INDEX_FILE}
        DEPENDS ${OPEN62541_PUBLIC_HEADER} ${OPEN62541_COPY_LIST}
        COMMAND ${DOXYGEN_EXECUTABLE} ${OPEN62541_DOXYFILE_OUT}
        MAIN_DEPENDENCY ${OPEN62541_DOXYFILE_OUT} ${OPEN62541_DOXYFILE_IN}
        COMMENT "Generating doxygen documentation for open62541..."
        VERBATIM
    )

    add_custom_target(doxygen-open62541 DEPENDS ${OPEN62541_DOXYGEN_INDEX_FILE})
    add_dependencies(
        doxygen-open62541
        open62541-generator-ids-ns0
        open62541-generator-namespace
        open62541-generator-statuscode
        open62541-generator-transport
        open62541-generator-types
    )
endif()
