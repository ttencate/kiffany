#ifndef BLOCK_H
#define BLOCK_H

#include <stdint.h>

typedef uint32_t Block;

enum {
	AIR_BLOCK = 0,
	STONE_BLOCK = 1
};

inline bool needsDrawing(Block block) {
	return block != AIR_BLOCK;
}

inline bool needsDrawing(Block back, Block front) {
	return back == STONE_BLOCK && front == AIR_BLOCK;
}

#endif
