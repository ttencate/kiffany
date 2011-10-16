#ifndef COORDS_H
#define COORDS_H

#include "maths.h"

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

inline int3 chunkIndexFromPosition(vec3 const &position) {
	return int3(floor(position / (float)CHUNK_SIZE));
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
