#ifndef BLOCK_H
#define BLOCK_H

#include <stdint.h>

typedef uint32_t Block;

enum {
	AIR_BLOCK = 0,
	STONE_BLOCK = 1
};

bool needsDrawing(Block block);
bool needsDrawing(Block back, Block front);

#endif
