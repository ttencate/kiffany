#ifndef RAYCASTER_H
#define RAYCASTER_H

#include "block.h"
#include "octree.h"
#include "maths.h"

class ChunkMap;

struct RaycastResult {

	enum Status {
		CUTOFF,
		HIT,
		INDETERMINATE
	};

	Status const status;
	vec3 const endPointInStartChunk;
	Block const block;

	static inline RaycastResult cutoff(vec3 startPointInStartChunk, vec3 direction, float length) {
		return RaycastResult(CUTOFF, startPointInStartChunk + length * direction, INVALID_BLOCK);
	}

	static inline RaycastResult hit(vec3 startPointInStartChunk, vec3 direction, float length, Block block) {
		return RaycastResult(HIT, startPointInStartChunk + length * direction, block);
	}

	static inline RaycastResult indeterminate(vec3 startPointInStartChunk, vec3 direction, float length) {
		return RaycastResult(INDETERMINATE, startPointInStartChunk + length * direction, INVALID_BLOCK);
	}

	private:

		inline RaycastResult(Status status, vec3 endPointInStartChunk, Block block)
		:
			status(status),
			endPointInStartChunk(endPointInStartChunk),
			block(block)
		{
		}

};

class Raycaster {

	ChunkMap const &chunkMap;
	float const cutoff;
	Block const mask;
	Block const maskedBlock;

	public:

		Raycaster(ChunkMap const &chunkMap, float cutoff, Block block, Block mask = BLOCK_MASK);

		RaycastResult operator()(int3 startChunkIndex, vec3 startPointInStartChunk, vec3 direction) const;

	private:

		inline OctreeConstPtr getOctreeOrNull(int3 index) const;
		inline bool isHitBlock(Block block) const { return (block & mask) == maskedBlock; }

};

#endif