#ifndef CHUNK_H
#define CHUNK_H

#include "chunkdata.h"
#include "gl.h"

#include <boost/scoped_ptr.hpp>

#include <vector>

class Chunk
:
	boost::noncopyable
{

	int3 const index;
	int3 const position;

	bool generated;
	bool tesselated;
	bool uploaded;

	ChunkData data;

	boost::scoped_ptr<std::vector<float> > vertexData;
	boost::scoped_ptr<std::vector<float> > normalData;

	GLBuffer vertexBuffer;
	GLBuffer normalBuffer;

	public:

		Chunk(int3 const &index);

		int3 const &getIndex() { return index; }
		int3 const &getPosition() { return position; }

		bool isGenerated() const { return generated; }
		void setGenerated() { generated = true; }

		void tesselate();

		ChunkData &getData() { return data; }

		void render();
	
	private:

		void upload();

};

#endif
