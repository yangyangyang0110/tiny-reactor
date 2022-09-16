include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/../src/
)

file(GLOB FILES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

foreach(FILE ${FILES})
  get_filename_component(target ${FILE} NAME_WE)
  set(target "example-${target}")

  add_executable(${target} ${FILE})
  target_compile_definitions(
    ${target}
    PRIVATE
    $<$<CONFIG:Debug>:MyDebug>
  )

  target_link_libraries(
    ${target}
    PRIVATE
    spdlog::spdlog
  )

  target_compile_options(
    ${target}
    PRIVATE
    -Werror
    $<$<CONFIG:Debug>:-g -O0>
    $<$<CONFIG:Release>:-O2>
  )

endforeach()

