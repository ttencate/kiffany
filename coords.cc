#include "coords.h"

const unsigned CHUNK_POWER = 7;
const unsigned CHUNK_SIZE = 1 << CHUNK_POWER;

const unsigned BLOCKS_PER_CHUNK = CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE;

const float CHUNK_RADIUS = CHUNK_SIZE * 0.5f * sqrtf(3.0f);
