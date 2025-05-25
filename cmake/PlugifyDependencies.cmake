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
include(FetchGit)

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
