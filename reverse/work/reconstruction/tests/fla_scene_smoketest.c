#include "cornball/fla_scene.h"
#include "cornball/random.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

static void assert_close(const char *label, float actual, float expected, float epsilon)
{
    if (fabsf(actual - expected) > epsilon) {
        fprintf(
            stderr,
            "%s mismatch: actual=%0.9f expected=%0.9f epsilon=%0.9f\n",
            label,
            actual,
            expected,
            epsilon
        );
        exit(1);
    }
}

static void assert_equal_u32(const char *label, unsigned actual, unsigned expected)
{
    if (actual != expected) {
        fprintf(stderr, "%s mismatch: actual=%u expected=%u\n", label, actual, expected);
        exit(1);
    }
}

int main(void)
{
    CornballRandom random;
    CornballRandom random_for_scene;
    CornballFlaScene scene;
    CornballFlaFrame frame;

    cornball_random_seed(&random, 1u);
    assert_equal_u32("rand[0]", cornball_lcg_rand15(&random), 41u);
    assert_equal_u32("rand[1]", cornball_lcg_rand15(&random), 18467u);
    assert_equal_u32("rand[2]", cornball_lcg_rand15(&random), 6334u);

    cornball_random_seed(&random_for_scene, 1u);
    cornball_fla_scene_clear(&scene);
    cornball_fla_scene_loader_pass(&scene);

    assert_equal_u32("texture_group_loaded", scene.texture_group_loaded, 1u);

    cornball_fla_scene_update(&scene, &random_for_scene);

    assert_close("p0.brightness", scene.state_block[0].brightness, 0.001251259f, 0.000001f);
    assert_close("p0.vel_x", scene.velocity_block[0].vel_x, 0.044509720f, 0.000001f);
    assert_close("p0.vel_y", scene.velocity_block[0].vel_y, 0.757991254f, 0.000001f);
    assert_close("p0.accel_y", scene.accel_block[0].accel_y, -0.131311074f, 0.000001f);

    cornball_fla_scene_build_frame(&scene, 1.0, &frame);

    assert_equal_u32("frame.quad_count", (unsigned)frame.quad_count, 500u);
    assert_close("frame.rotation", frame.rotation_degrees, 192.528305054f, 0.000100f);
    assert_close("frame.p0.grayscale", frame.quads[0].grayscale, 0.003753777f, 0.000001f);
    assert_close("frame.p0.translate_z", frame.quads[0].translate_z, 0.0f, 0.000001f);

    cornball_fla_scene_update(&scene, &random_for_scene);

    assert_close("p1.brightness.live", scene.state_block[1].brightness, 0.526508391f, 0.000001f);
    assert_close("p1.pos_x.live", scene.state_block[1].pos_x, -0.014088870f, 0.000001f);
    assert_close("p1.pos_y.live", scene.state_block[1].pos_y, 0.805087447f, 0.000001f);
    assert_close("p1.vel_y.live", scene.velocity_block[1].vel_y, 0.660693049f, 0.000001f);

    return 0;
}
