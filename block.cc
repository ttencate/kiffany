#include "block.h"

bool needsDrawing(Block block) {
	return block != AIR_BLOCK;
}

bool needsDrawing(Block back, Block front) {
	return back == STONE_BLOCK && front == AIR_BLOCK;
}
