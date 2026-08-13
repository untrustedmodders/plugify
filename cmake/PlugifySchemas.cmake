# Compiles the manifest JSON schemas into the library instead of reading them from
# disk at runtime. They describe the same contract as the parser, so shipping them
# as separate files lets the two drift, and a relative path would resolve against
# the host process's working directory, which is not ours to control.
#
# The files under schemas/ remain the canonical, hand-edited source - this only
# copies them into the build, so the two can never disagree.
#
# Every schemas/*.schema.json becomes one constant named after the file, so adding
# a schema needs no change here: plugin.schema.json -> plugify::schemas::plugin.

# CONFIGURE_DEPENDS re-runs the glob at build time, so a schema that is added or
# removed is picked up without reconfiguring by hand.
file(GLOB PLUGIFY_SCHEMA_FILES CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/schemas/*.schema.json")

if(NOT PLUGIFY_SCHEMA_FILES)
    message(FATAL_ERROR "No schemas found in ${CMAKE_CURRENT_SOURCE_DIR}/schemas")
endif()

# Keep the generated header stable across machines, since glob order is not.
list(SORT PLUGIFY_SCHEMA_FILES)

# The glob notices files appearing and disappearing, but not their contents being
# edited, and file(READ) registers no dependency of its own. Without this a schema
# edit would leave a stale header in the build tree.
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS ${PLUGIFY_SCHEMA_FILES})

set(PLUGIFY_SCHEMA_DECLARATIONS "")

foreach(PLUGIFY_SCHEMA_FILE IN LISTS PLUGIFY_SCHEMA_FILES)
    get_filename_component(PLUGIFY_SCHEMA_BASENAME "${PLUGIFY_SCHEMA_FILE}" NAME)

    # plugin.schema.json -> plugin; anything a file name allows but an identifier
    # does not (a dash, say) becomes an underscore.
    string(REGEX REPLACE "\\.schema\\.json$" "" PLUGIFY_SCHEMA_NAME "${PLUGIFY_SCHEMA_BASENAME}")
    string(MAKE_C_IDENTIFIER "${PLUGIFY_SCHEMA_NAME}" PLUGIFY_SCHEMA_NAME)

    file(READ "${PLUGIFY_SCHEMA_FILE}" PLUGIFY_SCHEMA_TEXT)

    # A raw string literal ends at its delimiter, so a schema containing one would
    # break out of the literal. Nothing has a reason to, but emitting code that is
    # silently wrong is worse than failing the build.
    string(FIND "${PLUGIFY_SCHEMA_TEXT}" ")PLUGIFY_SCHEMA\"" PLUGIFY_SCHEMA_COLLISION)
    if(NOT PLUGIFY_SCHEMA_COLLISION EQUAL -1)
        message(FATAL_ERROR "${PLUGIFY_SCHEMA_BASENAME} contains the raw string delimiter )PLUGIFY_SCHEMA\"")
    endif()

    if(NOT PLUGIFY_SCHEMA_DECLARATIONS STREQUAL "")
        string(APPEND PLUGIFY_SCHEMA_DECLARATIONS "\n\n")
    endif()

    string(APPEND PLUGIFY_SCHEMA_DECLARATIONS
            "\t// ${PLUGIFY_SCHEMA_BASENAME}\n"
            "\tinline constexpr std::string_view ${PLUGIFY_SCHEMA_NAME} ="
            " R\"PLUGIFY_SCHEMA(${PLUGIFY_SCHEMA_TEXT})PLUGIFY_SCHEMA\";")

    message(VERBOSE "Embedding ${PLUGIFY_SCHEMA_BASENAME} as plugify::schemas::${PLUGIFY_SCHEMA_NAME}")
endforeach()

configure_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/cmake/plugifySchemas.hpp.in"
        "${CMAKE_CURRENT_BINARY_DIR}/generated/plugify/schemas.hpp"
        @ONLY
)
