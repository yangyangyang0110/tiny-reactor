cmake_minimum_required(VERSION 3.23 FATAL_ERROR)

project(tiny-reactor VERSION 1.0.0 LANGUAGES CXX C)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
include(external/spdlog)

add_subdirectory(src)

add_subdirectory(examples)

include(CTest)
enable_testing()
add_subdirectory(tests)

