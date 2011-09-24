#ifndef CHUNK_H
#define CHUNK_H

#include "gl.h"

#include "chunkdata.h"

class TerrainGenerator;

class Chunk
:
	boost::noncopyable
{

	int3 const index;
	int3 const position;

	ChunkData data;

	GLBuffer vertexBuffer;
	GLBuffer normalBuffer;

	void fillBuffers();

	public:

		Chunk(int3 const &index, TerrainGenerator &terrainGenerator);

		void render() const;

};

#endif
