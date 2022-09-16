add_executable(simplify_web main.cpp)

target_compile_definitions(
        simplify_web
        PRIVATE
        $<$<CONFIG:Debug>:MyDebug>
)

target_link_libraries(
        simplify_web
        PRIVATE
        spdlog::spdlog
)

target_compile_options(
        simplify_web
        PRIVATE
        -Werror
        $<$<CONFIG:Debug>:-g -O0>
        $<$<CONFIG:Release>:-O2>
)
