#ifndef CHUNK_H
#define CHUNK_H

#include "chunkdata.h"
#include "gl.h"

#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>

class ChunkBuffers
:
	boost::noncopyable
{
	GLBuffer vertexBuffer;
	GLBuffer normalBuffer;

	public:

		GLBuffer const &getVertexBuffer() const { return vertexBuffer; }
		GLBuffer &getVertexBuffer() { return vertexBuffer; }
		GLBuffer const &getNormalBuffer() const { return normalBuffer; }
		GLBuffer &getNormalBuffer() { return normalBuffer; }
};

class Chunk
:
	boost::noncopyable
{

	int3 const index;
	int3 const position;

	boost::scoped_ptr<ChunkData> data;
	boost::scoped_ptr<ChunkGeometry> geometry;
	boost::scoped_ptr<ChunkBuffers> buffers;

	public:

		Chunk(int3 const &index);

		int3 const &getIndex() { return index; }
		int3 const &getPosition() { return position; }

		void setData(ChunkData *data);
		void setGeometry(ChunkGeometry *geometry);
		void setBuffers(ChunkBuffers *buffers);

		bool canRender() const;
		void render();
	
};

void upload(ChunkGeometry const &geometry, ChunkBuffers *buffers);
void render(ChunkBuffers const &buffers);

typedef boost::shared_ptr<Chunk> ChunkPtr;

#endif
