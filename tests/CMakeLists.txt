
include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/../src/
)

add_executable(utest utest.cpp)
add_test(NAME utest
         COMMAND $<TARGET_FILE:utest>)
target_link_libraries(
  utest
  PRIVATE
  spdlog::spdlog
  pthread
)

