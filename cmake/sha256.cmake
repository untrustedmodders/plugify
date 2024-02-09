include(FetchContent)

message(STATUS "Pulling and configuring sha256")

FetchContent_Declare(
		sha256
		GIT_REPOSITORY https://github.com/qubka/SHA256.git
		GIT_TAG master
		GIT_SHALLOW TRUE
)

FetchContent_MakeAvailable(sha256)