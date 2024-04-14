include(FetchContent)

message(STATUS "Pulling and configuring glaze")

FetchContent_Declare(
		glaze
		GIT_REPOSITORY https://github.com/stephenberry/glaze.git
		GIT_TAG 10b9bad3974536b2965409f85b869ba8d1a815a9
)

FetchContent_MakeAvailable(glaze)