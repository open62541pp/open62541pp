if(NOT UAPP_INTERNAL_OPEN62541)
    message(FATAL_ERROR "Option UAPP_INTERNAL_OPEN62541 must be enabled to build documentation.")
endif()

find_package(Doxygen REQUIRED dot)
find_package(Python3 REQUIRED Interpreter)

# --------------------------------------- Doxygen open62541 -------------------------------------- #

set(doxygen_open62541_input_dir  ${CMAKE_CURRENT_BINARY_DIR}/open62541)  # generated files are copied and processed here
set(doxygen_open62541_output_dir ${PROJECT_BINARY_DIR}/doxygen_open62541)
set(doxygen_open62541_html_dir   ${PROJECT_BINARY_DIR}/doc_html/open62541)  # subdirectory of main documentation
set(doxygen_open62541_tagfile    ${doxygen_open62541_output_dir}/open62541.tag)
file(MAKE_DIRECTORY ${doxygen_open62541_input_dir})
file(MAKE_DIRECTORY ${doxygen_open62541_output_dir})
file(MAKE_DIRECTORY ${doxygen_open62541_html_dir})

get_target_property(open62541_version     open62541::open62541 VERSION)
get_target_property(open62541_source_dir  open62541::open62541 SOURCE_DIR)
get_target_property(open62541_binary_dir  open62541::open62541 BINARY_DIR)
get_target_property(open62541_include_dir open62541::open62541 INTERFACE_INCLUDE_DIRECTORIES)

configure_file(
    Doxyfile_open62541.in
    ${doxygen_open62541_output_dir}/Doxyfile
)
# copy README to directory with generated files (prevent long paths in doxygen file list)
configure_file(
    ${open62541_source_dir}/README.md
    ${doxygen_open62541_input_dir}/README.md
    COPYONLY
)

# copy all public interface files to a new folder (to strip rst comments and keywords)
list(APPEND open62541_copy_list "")
foreach(dir ${open62541_include_dir})
    string(REGEX REPLACE "\\$<[a-zA-Z_]+:(.+)>" "\\1" dir_stripped ${dir})  # strip generator expression
    string(REPLACE "${open62541_source_dir}" ${doxygen_open62541_input_dir} dir_target "${dir_stripped}")
    string(REPLACE "${open62541_binary_dir}" ${doxygen_open62541_input_dir} dir_target "${dir_target}")

    add_custom_command(
        OUTPUT ${dir_target}
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${dir_stripped} ${dir_target}
        COMMAND Python3::Interpreter ${CMAKE_CURRENT_SOURCE_DIR}/prepare_open62541_header.py ${dir_target}
        COMMENT "Copy ${dir_stripped} -> ${dir_target} and prepare for Doxygen"
        VERBATIM
    )
    list(APPEND open62541_copy_list ${dir_target})
endforeach()

add_custom_command(
    OUTPUT ${doxygen_open62541_html_dir}/index.html  # as dummy dependency for target
    COMMAND Doxygen::doxygen
    COMMENT "Build open62541 Doxygen documentation"
    DEPENDS
        Doxyfile_open62541.in
        DoxygenLayout_open62541.xml
        open62541-generator-ids-ns0
        open62541-generator-namespace
        open62541-generator-statuscode
        open62541-generator-transport
        open62541-generator-types
        ${open62541_copy_list}
    WORKING_DIRECTORY ${doxygen_open62541_output_dir}
)
add_custom_target(
    open62541pp_doc_open62541
    DEPENDS ${doxygen_open62541_html_dir}/index.html
)

# -------------------------------------- Doxygen open62541++ ------------------------------------- #

set(doxygen_output_dir ${PROJECT_BINARY_DIR}/doxygen)
set(doxygen_html_dir   ${PROJECT_BINARY_DIR}/doc_html)
file(MAKE_DIRECTORY ${doxygen_output_dir})
file(MAKE_DIRECTORY ${doxygen_html_dir})

configure_file(
    Doxyfile.in
    ${doxygen_output_dir}/Doxyfile
)

file(GLOB_RECURSE open62541pp_sources "${PROJECT_SOURCE_DIR}/src/*.cpp")
file(GLOB_RECURSE open62541pp_headers "${PROJECT_SOURCE_DIR}/include/*.h")
file(GLOB_RECURSE open62541pp_examples "${PROJECT_SOURCE_DIR}/examples/*.cpp")

add_custom_command(
    OUTPUT ${doxygen_html_dir}/index.html  # as dummy dependency for target
    COMMAND Doxygen::doxygen
    COMMENT "Build open62541pp Doxygen documentation"
    DEPENDS
        Doxyfile.in
        DoxygenLayout.xml
        example.dox
        ${PROJECT_SOURCE_DIR}/README.md
        ${open62541pp_sources}
        ${open62541pp_headers}
        ${open62541pp_examples}
    WORKING_DIRECTORY ${doxygen_output_dir}
)
add_custom_target(
    open62541pp_doc
    ALL  # add to default build target
    DEPENDS ${doxygen_html_dir}/index.html
)
add_dependencies(open62541pp_doc open62541pp_doc_open62541)
