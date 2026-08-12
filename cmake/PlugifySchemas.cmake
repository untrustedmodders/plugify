# Compiles the manifest JSON schemas into the library instead of reading them from
# disk at runtime. They describe the same contract as the parser, so shipping them
# as separate files lets the two drift, and a relative path would resolve against
# the host process's working directory, which is not ours to control.
#
# The files under schemas/ remain the canonical, hand-edited source - this only
# copies them into the build, so the two can never disagree.

set(PLUGIFY_SCHEMA_FILES
        "${CMAKE_CURRENT_SOURCE_DIR}/schemas/plugin.schema.json"
        "${CMAKE_CURRENT_SOURCE_DIR}/schemas/language-module.schema.json"
)

# configure_file() runs at configure time and file(READ) registers no dependency,
# so without this a schema edit would leave a stale header in the build tree.
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS ${PLUGIFY_SCHEMA_FILES})

list(GET PLUGIFY_SCHEMA_FILES 0 PLUGIFY_PLUGIN_SCHEMA_FILE)
list(GET PLUGIFY_SCHEMA_FILES 1 PLUGIFY_MODULE_SCHEMA_FILE)

file(READ "${PLUGIFY_PLUGIN_SCHEMA_FILE}" PLUGIFY_PLUGIN_SCHEMA)
file(READ "${PLUGIFY_MODULE_SCHEMA_FILE}" PLUGIFY_MODULE_SCHEMA)

# A raw string literal ends at its delimiter, so a schema containing one would
# break out of the literal. Nothing has a reason to, but emitting code that is
# silently wrong is worse than failing the build.
foreach(PLUGIFY_SCHEMA IN ITEMS PLUGIFY_PLUGIN_SCHEMA PLUGIFY_MODULE_SCHEMA)
    string(FIND "${${PLUGIFY_SCHEMA}}" ")PLUGIFY_SCHEMA\"" PLUGIFY_SCHEMA_COLLISION)
    if(NOT PLUGIFY_SCHEMA_COLLISION EQUAL -1)
        message(FATAL_ERROR "${PLUGIFY_SCHEMA} contains the raw string delimiter )PLUGIFY_SCHEMA\"")
    endif()
endforeach()

configure_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/cmake/plugifySchemas.hpp.in"
        "${CMAKE_CURRENT_BINARY_DIR}/generated/plugify/schemas.hpp"
        @ONLY
)
