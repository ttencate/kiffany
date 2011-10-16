#include "coords.h"

const unsigned CHUNK_SIZE = 128;

const unsigned BLOCKS_PER_CHUNK = CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE;

const float CHUNK_RADIUS = CHUNK_SIZE * 0.5f * sqrtf(3.0f);
