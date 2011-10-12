#ifndef CHUNK_H
#define CHUNK_H

#include "chunkdata.h"
#include "gl.h"

#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>

class ChunkGeometry
:
	boost::noncopyable
{
	std::vector<short> vertexData;
	std::vector<char> normalData;

	public:

		std::vector<short> const &getVertexData() const { return vertexData; }
		std::vector<short> &getVertexData() { return vertexData; }
		std::vector<char> const &getNormalData() const { return normalData; }
		std::vector<char> &getNormalData() { return normalData; }

		bool isEmpty() const;

};

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

void upload(ChunkGeometry const &geometry, ChunkBuffers *buffers);
void render(ChunkBuffers const &buffers);

class Chunk
:
	boost::noncopyable
{

	int3 const index;
	int3 const position;

	enum State {
		NEW,
		GENERATING,
		GENERATED,
		TESSELATED,
		UPLOADED
	};
	State state;

	boost::scoped_ptr<ChunkData> data;
	boost::scoped_ptr<ChunkGeometry> geometry[6];
	boost::scoped_ptr<ChunkBuffers> buffers[6];

	public:

		Chunk(int3 const &index);

		int3 const &getIndex() { return index; }
		int3 const &getPosition() { return position; }

		void generating() { state = GENERATING; }
		void setData(ChunkData *data);

		bool needsGenerating() const;
		void render();

	private:

		void tesselate();
		void upload();
	
};

typedef boost::shared_ptr<Chunk> ChunkPtr;

#endif
