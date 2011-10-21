#ifndef RAYCASTER_H
#define RAYCASTER_H

#include "block.h"
#include "maths.h"

class ChunkMap;

struct RaycastResult {

	enum Status {
		CUTOFF,
		HIT,
		UNKNOWN
	};

	Status status;
	float length;
	vec3 end;
	Block block;

};

class Raycaster {

	ChunkMap const &chunkMap;
	float const cutoff;
	Block const mask;
	Block const maskedBlock;

	public:

		Raycaster(ChunkMap const &chunkMap, float cutoff, Block block, Block mask);

		RaycastResult operator()(vec3 origin, vec3 direction) const;

	private:

		inline bool isHitBlock(Block block) const { return (block & mask) == maskedBlock; }

};

#endif
