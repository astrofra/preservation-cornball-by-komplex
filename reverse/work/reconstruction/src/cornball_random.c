#include "cornball/random.h"

enum {
    CORNBALL_LCG_MULTIPLIER = 214013u,
    CORNBALL_LCG_INCREMENT = 2531011u,
    CORNBALL_LCG_OUTPUT_MASK = 0x7fffu
};

static const float kRand15UnitScale = 1.0f / 32767.0f;

void cornball_random_seed(CornballRandom *random, uint32_t seed)
{
    random->state = seed;
}

uint16_t cornball_lcg_rand15(CornballRandom *random)
{
    random->state = random->state * CORNBALL_LCG_MULTIPLIER + CORNBALL_LCG_INCREMENT;
    return (uint16_t)((random->state >> 16) & CORNBALL_LCG_OUTPUT_MASK);
}

float cornball_rand15_unit(CornballRandom *random)
{
    return (float)cornball_lcg_rand15(random) * kRand15UnitScale;
}
