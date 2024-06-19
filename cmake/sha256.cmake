include(FetchContent)

message(STATUS "Pulling and configuring sha256")

FetchContent_Declare(
        sha256
        GIT_REPOSITORY https://github.com/System-Glitch/SHA256.git
        GIT_TAG 1de731e3762ccb9d50ddd3c8f86ccd36fdb51aad
)

FetchContent_MakeAvailable(sha256)