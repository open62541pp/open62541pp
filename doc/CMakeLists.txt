set(DOC_HTML_DIR ${PROJECT_BINARY_DIR}/doc_html)
make_directory(${DOC_HTML_DIR})

# DOXYGEN
find_package(Doxygen)
if(DOXYGEN_FOUND)
    message(STATUS "Found doxygen: ${DOXYGEN_EXECUTABLE}")
endif()

# configure Doxyfile.in
set(DOXYGEN_INPUT_DIR  ${PROJECT_SOURCE_DIR}/include/open62541pp)
set(DOXYGEN_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/doxygen)
set(DOXYGEN_INDEX_FILE ${DOXYGEN_OUTPUT_DIR}/xml/index.xml)
set(DOXYFILE_IN        ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in)
set(DOXYFILE_OUT       ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)

configure_file(${DOXYFILE_IN} ${DOXYFILE_OUT} @ONLY)

# find all public headers (as dependency for doxygen build)
get_target_property(OPEN62541PP_PUBLIC_HEADER_DIR open62541pp INTERFACE_INCLUDE_DIRECTORIES)
file(GLOB_RECURSE OPEN62541PP_PUBLIC_HEADERS ${OPEN62541PP_PUBLIC_HEADER_DIR}/*.h)

add_custom_command(OUTPUT ${DOXYGEN_INDEX_FILE}
                   DEPENDS ${OPEN62541PP_PUBLIC_HEADERS}
                   COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYFILE_OUT}
                   MAIN_DEPENDENCY ${DOXYFILE_OUT} ${DOXYFILE_IN}
                   COMMENT "Generating doxygen documentation..."
                   VERBATIM)

# SPHINX
find_program(SPHINX_BUILD
             NAMES sphinx-build
             DOC "Sphinx documentation generator")
if(SPHINX_BUILD)
    message(STATUS "Found sphinx-build: ${SPHINX_EXECUTABLE}")
endif()

set(SPHINX_CONF_FILE ${CMAKE_CURRENT_SOURCE_DIR}/conf.py)
add_custom_target(doc-html
                  COMMAND ${SPHINX_BUILD} -b html
                          # tell breathe where to find the Doxygen output
                          -Dbreathe_projects.open62541pp=${DOXYGEN_OUTPUT_DIR}/xml
                          ${CMAKE_CURRENT_SOURCE_DIR} ${DOC_HTML_DIR}
                  DEPENDS ${SPHINX_BUILD} ${SPHINX_CONF_FILE} ${DOXYGEN_INDEX_FILE}
                  COMMENT "Building HTML documentation with Sphinx")