if(UNIX)
    # should use LINK_OPTIONS instead of LINK_LIBRARIES, if we can use cmake v3.14+
    try_compile(COMPILER_SUPPORTS_VERSION_SCRIPT
            ${CMAKE_CURRENT_BINARY_DIR}
            SOURCES ${CMAKE_CURRENT_LIST_DIR}/vcheck.c
            LINK_LIBRARIES "-Wl,--version-script=${CMAKE_CURRENT_LIST_DIR}/vcheck.ver")
endif()