include(FetchContent)

message(STATUS "Pulling and configuring glaze")

FetchContent_Declare(
		glaze
		GIT_REPOSITORY https://github.com/stephenberry/glaze.git
		GIT_TAG main
		GIT_SHALLOW TRUE
)

FetchContent_MakeAvailable(glaze)