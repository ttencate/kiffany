#ifndef CHUNK_H
#define CHUNK_H

#include "gl.h"

#include "chunkdata.h"

class Chunk
:
	boost::noncopyable
{

	int3 const index;
	int3 const position;

	bool generated;
	bool tesselated;

	ChunkData data;

	GLBuffer vertexBuffer;
	GLBuffer normalBuffer;

	public:

		Chunk(int3 const &index);

		int3 const &getIndex() { return index; }
		int3 const &getPosition() { return position; }

		bool isGenerated() const { return generated; }
		void setGenerated() { generated = true; }

		ChunkData &getData() { return data; }

		void render();
	
	private:

		void tesselate();

};

#endif
