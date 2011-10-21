#include "raycaster.h"

#include "chunkmap.h"

Raycaster::Raycaster(ChunkMap const &chunkMap, float cutoff, Block block, Block mask)
:
	chunkMap(chunkMap),
	cutoff(cutoff),
	mask(mask),
	maskedBlock(mask & block)
{
}

RaycastResult Raycaster::operator()(vec3 origin, vec3 direction) const {
	RaycastResult result;
	result.status = RaycastResult::HIT;
	result.length = 100.0f;
	result.end = origin + result.length * direction;
	result.block = STONE_BLOCK;
	return result;
}
