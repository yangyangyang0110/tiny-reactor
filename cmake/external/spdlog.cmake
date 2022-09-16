include(FetchContent)
FetchContent_Declare(
    spdlog
    GIT_REPOSITORY https://github.com/gabime/spdlog.git
    GIT_TAG v1.10.0
    SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/spdlog
    BINARY_DIR ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/install/spdlog
)
FetchContent_MakeAvailable(spdlog)
