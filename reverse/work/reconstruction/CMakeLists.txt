cmake_minimum_required(VERSION 3.30)

project(cornball_reconstruction C)

option(CORNBALL_BUILD_TESTS "Build source-reconstruction smoke tests" ON)

add_library(
    cornball_reconstruction
    STATIC
    src/cornball_capture.c
    src/cornball_demo_script.c
    src/cornball_fla_scene.c
    src/cornball_fla_gl.c
    src/cornball_intro_scene.c
    src/cornball_intro_gl.c
    src/cornball_kaar_scene.c
    src/cornball_kaar_gl.c
    src/cornball_s_pair_scene.c
    src/cornball_s_pair_gl.c
    src/cornball_surf_scene.c
    src/cornball_surf_gl.c
    src/cornball_random.c
)

target_include_directories(cornball_reconstruction PUBLIC include)
target_compile_features(cornball_reconstruction PUBLIC c_std_99)

if(MSVC)
    target_compile_options(cornball_reconstruction PRIVATE /W4)
    target_compile_definitions(cornball_reconstruction PRIVATE _CRT_SECURE_NO_WARNINGS)
else()
    target_compile_options(cornball_reconstruction PRIVATE -Wall -Wextra -Wpedantic)
endif()

include(CTest)

if(CORNBALL_BUILD_TESTS)
    add_executable(
        demo_script_smoketest
        tests/demo_script_smoketest.c
    )

    target_link_libraries(demo_script_smoketest PRIVATE cornball_reconstruction)

    if(NOT MSVC)
        target_link_libraries(demo_script_smoketest PRIVATE m)
    endif()

    add_test(NAME demo_script_smoketest COMMAND demo_script_smoketest)

    add_executable(
        fla_scene_smoketest
        tests/fla_scene_smoketest.c
    )

    target_link_libraries(fla_scene_smoketest PRIVATE cornball_reconstruction)

    if(NOT MSVC)
        target_link_libraries(fla_scene_smoketest PRIVATE m)
    endif()

    add_test(NAME fla_scene_smoketest COMMAND fla_scene_smoketest)

    add_executable(
        intro_scene_smoketest
        tests/intro_scene_smoketest.c
    )

    target_link_libraries(intro_scene_smoketest PRIVATE cornball_reconstruction)

    if(NOT MSVC)
        target_link_libraries(intro_scene_smoketest PRIVATE m)
    endif()

    add_test(NAME intro_scene_smoketest COMMAND intro_scene_smoketest)

    add_executable(
        kaar_scene_smoketest
        tests/kaar_scene_smoketest.c
    )

    target_link_libraries(kaar_scene_smoketest PRIVATE cornball_reconstruction)

    if(NOT MSVC)
        target_link_libraries(kaar_scene_smoketest PRIVATE m)
    endif()

    add_test(NAME kaar_scene_smoketest COMMAND kaar_scene_smoketest)

    add_executable(
        surf_scene_smoketest
        tests/surf_scene_smoketest.c
    )

    target_link_libraries(surf_scene_smoketest PRIVATE cornball_reconstruction)

    if(NOT MSVC)
        target_link_libraries(surf_scene_smoketest PRIVATE m)
    endif()

    add_test(NAME surf_scene_smoketest COMMAND surf_scene_smoketest)

    add_executable(
        s_pair_scene_smoketest
        tests/s_pair_scene_smoketest.c
    )

    target_link_libraries(s_pair_scene_smoketest PRIVATE cornball_reconstruction)

    if(NOT MSVC)
        target_link_libraries(s_pair_scene_smoketest PRIVATE m)
    endif()

    add_test(NAME s_pair_scene_smoketest COMMAND s_pair_scene_smoketest)
endif()

if(WIN32)
    set(
        CORNBALL_ORIGINAL_MIDAS_DLL
        "${CMAKE_CURRENT_SOURCE_DIR}/../../../original/cornball/MIDAS06.DLL"
    )

    add_executable(
        cornball_demo_replay
        src/cornball_demo_replay_win32.c
    )

    target_link_libraries(
        cornball_demo_replay
        PRIVATE
        cornball_reconstruction
        opengl32
        gdi32
        user32
    )

    if((CMAKE_SIZEOF_VOID_P EQUAL 4) AND EXISTS "${CORNBALL_ORIGINAL_MIDAS_DLL}")
        add_custom_command(
            TARGET cornball_demo_replay
            POST_BUILD
            COMMAND
                ${CMAKE_COMMAND} -E copy_if_different
                "${CORNBALL_ORIGINAL_MIDAS_DLL}"
                "$<TARGET_FILE_DIR:cornball_demo_replay>/MIDAS06.DLL"
        )
    endif()

    if(CORNBALL_BUILD_TESTS)
        add_test(
            NAME cornball_demo_replay_smoketest
            COMMAND cornball_demo_replay --hidden --frames 120 --position-seconds 0.05
        )
    endif()
endif()
