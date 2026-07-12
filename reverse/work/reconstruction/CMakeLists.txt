cmake_minimum_required(VERSION 3.30)

project(cornball_reconstruction C)

option(CORNBALL_BUILD_TESTS "Build source-reconstruction smoke tests" ON)

add_library(
    cornball_reconstruction
    STATIC
    src/cornball_fla_scene.c
    src/cornball_random.c
)

target_include_directories(cornball_reconstruction PUBLIC include)
target_compile_features(cornball_reconstruction PUBLIC c_std_99)

if(MSVC)
    target_compile_options(cornball_reconstruction PRIVATE /W4)
else()
    target_compile_options(cornball_reconstruction PRIVATE -Wall -Wextra -Wpedantic)
endif()

include(CTest)

if(CORNBALL_BUILD_TESTS)
    add_executable(
        fla_scene_smoketest
        tests/fla_scene_smoketest.c
    )

    target_link_libraries(fla_scene_smoketest PRIVATE cornball_reconstruction)

    if(NOT MSVC)
        target_link_libraries(fla_scene_smoketest PRIVATE m)
    endif()

    add_test(NAME fla_scene_smoketest COMMAND fla_scene_smoketest)
endif()
