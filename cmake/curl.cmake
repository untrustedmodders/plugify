include(FetchContent)

message(STATUS "Pulling and configuring curl")

FetchContent_Declare(
		curl
		GIT_REPOSITORY https://github.com/curl/curl.git
		GIT_TAG curl-8_6_0
)

set(BUILD_TESTING OFF CACHE BOOL "Turn off testing" FORCE)
set(BUILD_CURL_EXE OFF CACHE BOOL "Turn off curl executable" FORCE)
if(PLUGIFY_BUILD_SHARED_CURL)
	set(BUILD_SHARED_LIBS ON CACHE BOOL "Build shared libraries")
	set(BUILD_STATIC_LIBS OFF CACHE BOOL "Build static libraries")
else()
	set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build shared libraries")
	set(BUILD_STATIC_LIBS ON CACHE BOOL "Build static libraries")
endif()

if(WIN32)
	set(CURL_USE_SCHANNEL ON CACHE BOOL "Use schannel to build libcurl" FORCE)
else()
	set(CURL_USE_OPENSSL ON CACHE BOOL "Use OpenSSL to build libcurl" FORCE)
endif()

FetchContent_MakeAvailable(curl)

set(CURL_FOUND TRUE)
set(CURL_LIBRARIES libcurl)
set(CURL_INCLUDE_DIRS
		${CURL_SOURCE_DIR}/include
		${CURL_BINARY_DIR}/include/curl)

if(COMPILER_SUPPORTS_FPIC)
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC")
endif()