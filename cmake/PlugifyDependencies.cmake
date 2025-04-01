# ------------------------------------------------------------------------------
# Not build deps in interface mode
if(NOT PLUGIFY_INTERFACE)
    # ------------------------------------------------------------------------------
    # Glaze
    if(PLUGIFY_USE_EXTERNAL_GLAZE)
        find_package(glaze REQUIRED)
    else()
        include(FetchGlaze)
    endif()
    target_link_libraries(${PROJECT_NAME} PRIVATE glaze::glaze)

    # ------------------------------------------------------------------------------
    # Http/Curl
    if(PLUGIFY_DOWNLOADER)
        if(WIN32)
            target_link_libraries(${PROJECT_NAME} PRIVATE winhttp.lib)
        else()
            if(PLUGIFY_USE_EXTERNAL_CURL)
                find_package(CURL REQUIRED)
                target_link_libraries(${PROJECT_NAME} PRIVATE CURL::libcurl)
                message(STATUS "Found CURL version: ${CURL_VERSION_STRING}")
                message(STATUS "Using CURL include dir(s): ${CURL_INCLUDE_DIRS}")
                message(STATUS "Using CURL lib(s): ${CURL_LIBRARIES}")
            else()
                include(FetchCurl)
                target_link_libraries(${PROJECT_NAME} PRIVATE ${CURL_LIBRARIES})
                target_include_directories(${PROJECT_NAME} PRIVATE ${CURL_INCLUDE_DIRS})
            endif()
        endif()
    endif()

    # ------------------------------------------------------------------------------
    # Miniz
    include(FetchMiniz)
    target_link_libraries(${PROJECT_NAME} PRIVATE miniz)
    set_property(TARGET miniz PROPERTY POSITION_INDEPENDENT_CODE ON)
endif()

# ------------------------------------------------------------------------------
# Format
cmake_push_check_state()

check_cxx_source_compiles("
    #include<format>
    #include<string>

    int main() {
        std::string a = std::format(\"{}.{}.{}\", \"Hello\", \"World\", \"C++\");
        return 0;
    }
    " COMPILER_SUPPORTS_FORMAT)

cmake_pop_check_state()

if(NOT COMPILER_SUPPORTS_FORMAT)
    if(PLUGIFY_USE_EXTERNAL_FMT)
        find_package(fmt REQUIRED)
    else()
        include(FetchFmt)
    endif()
    target_link_libraries(${PROJECT_NAME} PRIVATE fmt::fmt-header-only)
endif()

# ------------------------------------------------------------------------------
# Stacktrace
cmake_push_check_state()

check_cxx_source_compiles("
    #include <stacktrace>

    int main() {
        auto trace = std::stacktrace::current();
        return 0;
    }
    " COMPILER_SUPPORTS_STACKTRACE)

cmake_pop_check_state()

if(NOT COMPILER_SUPPORTS_STACKTRACE)
    if(PLUGIFY_USE_EXTERNAL_CPPTRACE)
        find_package(cpptrace REQUIRED)
    else()
        include(FetchCppTrace)
    endif()
endif()

# ------------------------------------------------------------------------------
# Git
find_package(Git QUIET)

if (Git_FOUND)
    # the commit's SHA1, and whether the building workspace was dirty or not
    # describe --match=NeVeRmAtCh --always --tags --abbrev=40 --dirty
    execute_process(COMMAND
            "${GIT_EXECUTABLE}" --no-pager describe --tags --always --dirty
            WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
            OUTPUT_VARIABLE GIT_SHA1
            ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE)

    # branch
    execute_process(
            COMMAND "${GIT_EXECUTABLE}" rev-parse --abbrev-ref HEAD
            WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
            OUTPUT_VARIABLE GIT_BRANCH
            OUTPUT_STRIP_TRAILING_WHITESPACE
    )

    execute_process(COMMAND
            "${GIT_EXECUTABLE}" config --get remote.origin.url
            WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
            OUTPUT_VARIABLE GIT_URL
            ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE)

    # the date of the commit
    execute_process(COMMAND
            "${GIT_EXECUTABLE}" log -1 --format=%ad --date=local
            WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
            OUTPUT_VARIABLE GIT_DATE
            ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE)

    execute_process(COMMAND
            "${GIT_EXECUTABLE}" describe --tags --abbrev=0
            WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
            OUTPUT_VARIABLE GIT_TAG
            ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE)

    # the subject of the commit
    execute_process(COMMAND
            "${GIT_EXECUTABLE}" log -1 --format=%s
            WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
            OUTPUT_VARIABLE GIT_COMMIT_SUBJECT
            ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE)

    # remove # from subject
    if (DEFINED ${GIT_COMMIT_SUBJECT})
        string(REGEX REPLACE "[\#\"]+"
                "" GIT_COMMIT_SUBJECT
                ${GIT_COMMIT_SUBJECT})
    endif()
else()
    message(STATUS "Git not installed")
    set(GIT_SHA1 "UNKNOWN")
    set(GIT_DATE "UNKNOWN")
    set(GIT_COMMIT_SUBJECT "UNKNOWN")
    set(GIT_BRANCH "UNKNOWN")
    set(GIT_URL "UNKNOWN")
    set(GIT_TAG "UNKNOWN")
endif()

string(REPLACE "#" "\#" GIT_COMMIT_SUBJECT "${GIT_COMMIT_SUBJECT}")
string(REPLACE ":" "\:" GIT_COMMIT_SUBJECT "${GIT_COMMIT_SUBJECT}")
string(REPLACE ";" "\;" GIT_COMMIT_SUBJECT "${GIT_COMMIT_SUBJECT}")

add_library(plugify-git INTERFACE)
target_compile_definitions(plugify-git INTERFACE
        PLUGIFY_GIT_COMMIT_HASH="${GIT_SHA1}"
        PLUGIFY_GIT_COMMIT_DATE="${GIT_DATE}"
        PLUGIFY_GIT_TAG="${GIT_TAG}"
        PLUGIFY_GIT_COMMIT_SUBJECT="${GIT_COMMIT_SUBJECT}"
        PLUGIFY_GIT_BRANCH="${GIT_BRANCH}"
        PLUGIFY_GIT_URL="${GIT_URL}"
)
target_link_libraries(${PROJECT_NAME} PUBLIC plugify-git)

# ------------------------------------------------------------------------------
# Exports
include(GenerateExportHeader)
generate_export_header(${PROJECT_NAME}
        BASE_NAME PLUGIFY
        EXPORT_MACRO_NAME PLUGIFY_API
        NO_EXPORT_MACRO_NAME PLUGIFY_PRIVATE
        EXPORT_FILE_NAME ${CMAKE_BINARY_DIR}/exports/${PROJECT_NAME}_export.h
        STATIC_DEFINE PLUGIFY_STATIC
)
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_BINARY_DIR}/exports)
