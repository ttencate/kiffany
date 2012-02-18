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

RaycastResult Raycaster::operator()(int3 const startChunkIndex, vec3 const startPointInStartChunk, const vec3 direction) const {
	int3 const startChunkPosition = chunkPositionFromIndex(startChunkIndex);

	float length = 0.0f;
	int3 currentPositionInStartChunk = blockPositionFromPoint(startPointInStartChunk);
	// The start point need not be in the start chunk.
	int3 currentChunkIndex = chunkIndexFromPosition(startChunkPosition + currentPositionInStartChunk);
	int3 currentChunkOffset = chunkPositionFromIndex(currentChunkIndex) - chunkPositionFromIndex(startChunkIndex);

	OctreeConstPtr octree = chunkMap.getOctreeOrNull(currentChunkIndex);
	if (!octree) {
		return RaycastResult::indeterminate(startPointInStartChunk, direction, length);
	}
	Block block;
	int3 base;
	unsigned size;
	octree->getBlock(currentPositionInStartChunk, &block, &base, &size);
	while (length < cutoff) {
		if (isHitBlock(block)) {
			return RaycastResult::hit(startPointInStartChunk, direction, length, block);
		}
		int3 const minInStartChunk = currentChunkOffset + base;
		int3 const maxInStartChunk = minInStartChunk + int3(size);
		vec3 const minInStartChunkF = vec3(minInStartChunk);
		vec3 const maxInStartChunkF = vec3(maxInStartChunk);
		// TODO optimize away direction.coord checks, use templated function,
		// reducing from 6 to 3 checks
		float t = std::numeric_limits<float>::infinity();
		int3 newPositionInStartChunk = currentPositionInStartChunk;
		float nt;
		if (direction.x < 0 && (nt = (minInStartChunkF.x - startPointInStartChunk.x) / direction.x) < t) {
			t = nt;
			newPositionInStartChunk = int3(
					minInStartChunk.x - 1,
					floor(startPointInStartChunk.y + t * direction.y),
					floor(startPointInStartChunk.z + t * direction.z));
		}
		if (direction.x > 0 && (nt = (maxInStartChunkF.x - startPointInStartChunk.x) / direction.x) < t) {
			t = nt;
			newPositionInStartChunk = int3(
					maxInStartChunk.x,
					floor(startPointInStartChunk.y + t * direction.y),
					floor(startPointInStartChunk.z + t * direction.z));
		}
		if (direction.y < 0 && (nt = (minInStartChunkF.y - startPointInStartChunk.y) / direction.y) < t) {
			t = nt;
			newPositionInStartChunk = int3(
					floor(startPointInStartChunk.x + t * direction.x),
					minInStartChunk.y - 1,
					floor(startPointInStartChunk.z + t * direction.z));
		}
		if (direction.y > 0 && (nt = (maxInStartChunkF.y - startPointInStartChunk.y) / direction.y) < t) {
			t = nt;
			newPositionInStartChunk = int3(
					floor(startPointInStartChunk.x + t * direction.x),
					maxInStartChunk.y,
					floor(startPointInStartChunk.z + t * direction.z));
		}
		if (direction.z < 0 && (nt = (minInStartChunkF.z - startPointInStartChunk.z) / direction.z) < t) {
			t = nt;
			newPositionInStartChunk = int3(
					floor(startPointInStartChunk.x + t * direction.x),
					floor(startPointInStartChunk.y + t * direction.y),
					minInStartChunk.z - 1);
		}
		if (direction.z > 0 && (nt = (maxInStartChunkF.z - startPointInStartChunk.z) / direction.z) < t) {
			t = nt;
			newPositionInStartChunk = int3(
					floor(startPointInStartChunk.x + t * direction.x),
					floor(startPointInStartChunk.y + t * direction.y),
					maxInStartChunk.z);
		}
		length = t;

		if (length >= cutoff) {
			// If the next chunk is unavailable, we still want to cut off rather than return INDETERMINATE.
			break;
		}

		// Due to roundoff in the floors' arguments above,
		// we might end up taking a step "backwards" and this can lead to cycles.
		// Ensure that we are never going backwards, using integer arithmetic only.
		if (direction.x < 0 && newPositionInStartChunk.x > currentPositionInStartChunk.x) {
			newPositionInStartChunk.x = currentPositionInStartChunk.x;
		}
		if (direction.x > 0 && newPositionInStartChunk.x < currentPositionInStartChunk.x) {
			newPositionInStartChunk.x = currentPositionInStartChunk.x;
		}
		if (direction.y < 0 && newPositionInStartChunk.y > currentPositionInStartChunk.y) {
			newPositionInStartChunk.y = currentPositionInStartChunk.y;
		}
		if (direction.y > 0 && newPositionInStartChunk.y < currentPositionInStartChunk.y) {
			newPositionInStartChunk.y = currentPositionInStartChunk.y;
		}
		if (direction.z < 0 && newPositionInStartChunk.z > currentPositionInStartChunk.z) {
			newPositionInStartChunk.z = currentPositionInStartChunk.z;
		}
		if (direction.z > 0 && newPositionInStartChunk.z < currentPositionInStartChunk.z) {
			newPositionInStartChunk.z = currentPositionInStartChunk.z;
		}
		currentPositionInStartChunk = newPositionInStartChunk;

		int3 newChunkIndex = chunkIndexFromPosition(startChunkPosition + currentPositionInStartChunk);
		if (newChunkIndex != currentChunkIndex) {
			currentChunkIndex = newChunkIndex;
			octree = chunkMap.getOctreeOrNull(currentChunkIndex);
			if (!octree) {
				return RaycastResult::indeterminate(startPointInStartChunk, direction, length);
			}
			currentChunkOffset = chunkPositionFromIndex(currentChunkIndex) - startChunkPosition;
		}

		octree->getBlock(currentPositionInStartChunk - currentChunkOffset, &block, &base, &size);
	}
	return RaycastResult::cutoff(startPointInStartChunk, direction, length);
}
