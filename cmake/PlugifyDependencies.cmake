# ------------------------------------------------------------------------------
# glaze
if(PLUGIFY_USE_EXTERNAL_GLAZE)
    find_package(glaze REQUIRED)
else()
    include(FetchGlaze)
endif()
target_link_libraries(${PROJECT_NAME} PRIVATE glaze::glaze)

# ------------------------------------------------------------------------------
# libsolv
if(PLUGIFY_USE_EXTERNAL_LIBSOLV)
    find_package(Libsolv REQUIRED)
else()
    include(FetchLibsolv)
endif()
target_link_libraries(${PROJECT_NAME} PRIVATE solv::libsolvext)

# ------------------------------------------------------------------------------
# asmjit
if(PLUGIFY_USE_EXTERNAL_ASMJIT)
    find_package(asmjit REQUIRED)
else()
    include(FetchAsmjit)
    target_include_directories(${PROJECT_NAME} PRIVATE ${ASMJIT_SRC})
endif()
target_link_libraries(${PROJECT_NAME} PRIVATE asmjit::asmjit)

# ------------------------------------------------------------------------------
# re2
if(PLUGIFY_USE_EXTERNAL_RE2)
    find_package(re2 REQUIRED)
else()
    include(FetchRe2)
endif()
target_link_libraries(${PROJECT_NAME} PRIVATE re2::re2)

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
# Crashpad
if(PLUGIFY_BUILD_CRASHPAD)
    include(FetchCrashPad)
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
