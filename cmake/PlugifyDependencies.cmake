# ------------------------------------------------------------------------------
# glaze
if(PLUGIFY_USE_EXTERNAL_GLAZE)
    find_package(glaze REQUIRED)
else()
    include(FetchGlaze)
endif()
target_link_libraries(${PROJECT_NAME} PRIVATE glaze::glaze)

include(FetchLibsolv)
target_link_libraries(${PROJECT_NAME} PUBLIC solv::libsolvext)

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
