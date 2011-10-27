#ifndef COORDS_H
#define COORDS_H

#include "maths.h"

extern const unsigned CHUNK_POWER;
extern const unsigned CHUNK_SIZE;

extern const unsigned BLOCKS_PER_CHUNK;

extern const float CHUNK_RADIUS;

inline vec3 blockMin(int3 const &pos) {
	return vec3(pos);
}

inline vec3 blockMax(int3 const &pos) {
	return vec3(pos) + vec3(1.0f);
}

inline vec3 blockCenter(int3 const &pos) {
	return vec3(pos.x + 0.5f, pos.y + 0.5f, pos.z + 0.5f);
}

inline int3 blockPositionFromPoint(vec3 const &pos) {
	return int3(pos);
}

inline int3 chunkIndexFromPosition(int3 const &position) {
	// Division breaks down for negative numbers, because the rounding is implementation-definded.
	// Technically, this solution is not kosher either: twos-complement notation is not guaranteed.
	return int3(position.x >> CHUNK_POWER, position.y >> CHUNK_POWER, position.z >> CHUNK_POWER);
}

inline int3 chunkIndexFromPoint(vec3 const &point) {
	return int3(floor(point / (float)CHUNK_SIZE));
}

inline int3 chunkPositionFromIndex(int3 const &index) {
	return int3((int)CHUNK_SIZE * index);
}

inline vec3 chunkMin(int3 const &index) {
	return (float)CHUNK_SIZE * vec3(index);
}

inline vec3 chunkMax(int3 const &index) {
	return (float)CHUNK_SIZE * vec3(index + 1);
}

inline vec3 chunkCenter(int3 const &index) {
	return (float)CHUNK_SIZE * (vec3(index) + 0.5f);
}

#endif
