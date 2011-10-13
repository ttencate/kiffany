#ifndef CHUNK_H
#define CHUNK_H

#include "chunkdata.h"
#include "gl.h"

#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>

#include <utility>

struct Range {
	unsigned begin;
	unsigned end;
	Range() : begin(0), end(0) { }
	Range(unsigned begin, unsigned end) : begin(begin), end(end) { }
	bool isEmpty() const { return end == begin; }
};

class Ranges {
	Range ranges[6];
	public:
		Range const &operator[](unsigned index) const { return ranges[index]; }
		Range &operator[](unsigned index) { return ranges[index]; }
};

class ChunkGeometry
:
	boost::noncopyable
{
	std::vector<short> vertexData;
	std::vector<char> normalData;

	Ranges ranges;

	public:

		std::vector<short> const &getVertexData() const { return vertexData; }
		std::vector<short> &getVertexData() { return vertexData; }
		std::vector<char> const &getNormalData() const { return normalData; }
		std::vector<char> &getNormalData() { return normalData; }
		Ranges const &getRanges() const { return ranges; }
		void setRanges(Ranges const &ranges) { this->ranges = ranges; }
		void setRange(unsigned index, Range const &range) { ranges[index] = range; }

		bool isEmpty() const { return vertexData.size() == 0; }

};

void tesselate(ChunkData const &data, int3 const &position, ChunkGeometry *geometry);

class ChunkBuffers
:
	boost::noncopyable
{
	GLBuffer vertexBuffer;
	GLBuffer normalBuffer;

	Ranges ranges;

	public:

		GLBuffer const &getVertexBuffer() const { return vertexBuffer; }
		GLBuffer &getVertexBuffer() { return vertexBuffer; }
		GLBuffer const &getNormalBuffer() const { return normalBuffer; }
		GLBuffer &getNormalBuffer() { return normalBuffer; }
		Ranges const &getRanges() const { return ranges; }
		void setRanges(Ranges const &ranges) { this->ranges = ranges; }

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
	boost::scoped_ptr<ChunkGeometry> geometry;
	boost::scoped_ptr<ChunkBuffers> buffers;

	public:

		Chunk(int3 const &index);

		int3 const &getIndex() const { return index; }
		int3 const &getPosition() const { return position; }

		void generating() { state = GENERATING; }
		void setData(ChunkData *data);
		void setGeometry(ChunkGeometry *geometry);

		bool needsGenerating() const;

		void render();

	private:

		void upload();
	
};

typedef boost::shared_ptr<Chunk> ChunkPtr;

#endif
