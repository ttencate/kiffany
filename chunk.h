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

typedef std::vector<short> VertexArray;
typedef std::vector<char> NormalArray;

class ChunkGeometry
:
	boost::noncopyable
{

	VertexArray vertexData;
	NormalArray normalData;

	Ranges ranges;

	public:

		VertexArray const &getVertexData() const { return vertexData; }
		VertexArray &getVertexData() { return vertexData; }
		NormalArray const &getNormalData() const { return normalData; }
		NormalArray &getNormalData() { return normalData; }
		Ranges const &getRanges() const { return ranges; }
		void setRanges(Ranges const &ranges) { this->ranges = ranges; }
		void setRange(unsigned index, Range const &range) { ranges[index] = range; }

		bool isEmpty() const { return vertexData.size() == 0; }

};

typedef boost::shared_ptr<ChunkGeometry> ChunkGeometryPtr;

struct NeighbourChunkData {
	ChunkDataPtr xn;
	ChunkDataPtr xp;
	ChunkDataPtr yn;
	ChunkDataPtr yp;
	ChunkDataPtr zn;
	ChunkDataPtr zp;
	bool isComplete() const { return xn && xp && yn && yp && zn && zp; }
};

void tesselate(ChunkDataPtr data, NeighbourChunkData const &neighbourData, int3 const &position, ChunkGeometryPtr geometry);

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

	public:

		enum State {
			NEW,
			GENERATING,
			GENERATED,
			TESSELATING,
			TESSELATED,
			UPLOADED
		};

	private:

		int3 const index;
		int3 const position;

		State state;

		ChunkDataPtr data;
		ChunkGeometryPtr geometry;
		boost::scoped_ptr<ChunkBuffers> buffers;

	public:

		Chunk(int3 const &index);

		int3 const &getIndex() const { return index; }
		int3 const &getPosition() const { return position; }

		State getState() const { return state; }

		void setGenerating();
		void setData(ChunkDataPtr data);
		void setTesselating();
		void setGeometry(ChunkGeometryPtr geometry);

		ChunkDataPtr getData() const { return data; }

		void render();

	private:

		void upload();
	
};

typedef boost::shared_ptr<Chunk> ChunkPtr;

#endif
