#ifndef CORNBALL_RANDOM_H
#define CORNBALL_RANDOM_H

#include <stdint.h>

typedef struct CornballRandom {
    uint32_t state;
} CornballRandom;

void cornball_random_seed(CornballRandom *random, uint32_t seed);
uint16_t cornball_lcg_rand15(CornballRandom *random);
float cornball_rand15_unit(CornballRandom *random);

#endif
