# Find open62541.
#
# This module defines:
#  open62541_INCLUDE_DIR, the include dir for open62541
#  open62541_LIBRARY, the open62541 library.
#  open62541_FOUND, If false, open62541 was not found.
#
# If you have open62541 installed in a non-standard place, you can define
# open62541_PREFIX to tell cmake where it is.

find_path(
    open62541_INCLUDE_DIR
    NAMES
        client.h
        nodeids.h
        plugin/accesscontrol_default.h
        server.h
        server_config_default.h
        types.h
        types_generated.h
        types_generated_handling.h
    HINTS /usr/include /usr/local/include ${open62541_PREFIX}/include
    PATH_SUFFIXES open62541
)

find_library(
    open62541_LIBRARY
    NAMES open62541
    HINTS /usr/lib /usr/local/lib /usr/lib/x86_64-linux-gnu/lib ${open62541_PREFIX}/lib
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
    open62541
    FOUND_VAR open62541_FOUND
    REQUIRED_VARS
        open62541_LIBRARY
        open62541_INCLUDE_DIR
    VERSION_VAR open62541_VERSION
)

mark_as_advanced(open62541_INCLUDE_DIR open62541_LIBRARY)

if(open62541_FOUND)
    message(STATUS "Found open62541: ${open62541_LIBRARY}, ${open62541_INCLUDE_DIR}")

    set(open62541_LIBRARIES ${open62541_LIBRARY})
    set(open62541_INCLUDE_DIRS ${open62541_INCLUDE_DIR})
endif()

if(open62541_FOUND AND NOT TARGET open62541::open62541)
    add_library(open62541::open62541 UNKNOWN IMPORTED)
    set_target_properties(
        open62541::open62541
        PROPERTIES
            IMPORTED_LOCATION "${open62541_LIBRARY}"
            INTERFACE_INCLUDE_DIRECTORIES "${open62541_INCLUDE_DIR}"
    )
endif()
