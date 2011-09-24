#ifndef CHUNK_H
#define CHUNK_H

#include "gl.h"

#include "chunkdata.h"

class TerrainGenerator;

class Chunk
:
	boost::noncopyable
{

	int3 const pos;

	ChunkData data;

	GLBuffer vertexBuffer;
	GLBuffer normalBuffer;

	public:

		Chunk(int3 const &pos, TerrainGenerator &terrainGenerator);

		void render() const;

};

#endif
