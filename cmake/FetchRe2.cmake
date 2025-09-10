include(FetchContent)

message(STATUS "Pulling and configuring re2")

# Abseil is a build requirement for re2.
set(ABSL_PROPAGATE_CXX_STD ON)
set(ABSL_ENABLE_INSTALL ON)

FetchContent_Declare(
		absl
		GIT_REPOSITORY ${PLUGIFY_ABSL_REPO}
		GIT_TAG ${PLUGIFY_ABSL_TAG}
		GIT_PROGRESS TRUE
		GIT_SHALLOW TRUE
		OVERRIDE_FIND_PACKAGE
)
FetchContent_MakeAvailable(absl)

# Download and build a known-good version of re2.
FetchContent_Declare(
		re2
		GIT_REPOSITORY ${PLUGIFY_RE2_REPO}
		GIT_TAG ${PLUGIFY_RE2_TAG}
		GIT_PROGRESS TRUE
		GIT_SHALLOW TRUE
		OVERRIDE_FIND_PACKAGE
)
FetchContent_MakeAvailable(re2)