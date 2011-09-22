#ifndef CHUNK_H
#define CHUNK_H

#include "glm/gtx/compatibility.hpp"

using glm::gtx::compatibility::int3;

extern const unsigned CHUNK_SIZE;

class Chunk {

	int3 const pos;

	unsigned vertexBuffer;
	unsigned normalBuffer;

	public:

		Chunk(int3 const &pos);

		void render();

};

#endif
