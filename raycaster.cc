#include "raycaster.h"

#include "chunkmap.h"

#include <iostream>

Raycaster::Raycaster(ChunkMap const &chunkMap, float cutoff, Block block, Block mask)
:
	chunkMap(chunkMap),
	cutoff(cutoff),
	mask(mask),
	maskedBlock(mask & block)
{
}

RaycastResult Raycaster::operator()(int3 chunkIndex, vec3 origin, vec3 direction) const {
	RaycastResult result;
	result.length = 0;
	result.endChunkIndex = chunkIndex;

	std::cout << "Raycasting\n";
	ChunkConstPtr chunk = chunkMap[chunkIndex];
	while (chunk) {
		OctreeConstPtr octree = chunk->getOctree();
		if (!octree) {
			result.status = RaycastResult::INDETERMINATE;
			break;
		}
		vec3 p = origin + result.length * direction;
		Block block;
		int3 base;
		unsigned size;
		octree->getBlock(blockPosition(p), &block, &base, &size);
		std::cout << "Got block " << block << " at " << glm::to_string(base) << " (size " << size << ")\n";
		if (isHitBlock(block)) {
			result.status = RaycastResult::HIT;
			result.block = block;
			result.end = p;
			break;
		} else {
			result.status = RaycastResult::CUTOFF;
			result.length = 100.0f;
			result.end = origin + result.length * direction;
			break;
		}
	}
	return result;
}
