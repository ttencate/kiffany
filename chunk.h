#ifndef CHUNK_H
#define CHUNK_H

#include "buffer.h"
#include "chunkdata.h"
#include "octree.h"
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

class ChunkGeometry {

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
		unsigned getNumQuads() const { return vertexData.size() / (3 * 4); };

		bool isEmpty() const { return vertexData.size() == 0; }

};

typedef boost::shared_ptr<ChunkGeometry> ChunkGeometryPtr;
typedef boost::shared_ptr<ChunkGeometry const> ChunkGeometryConstPtr;

struct NeighbourOctrees {
	OctreeConstPtr xn;
	OctreeConstPtr xp;
	OctreeConstPtr yn;
	OctreeConstPtr yp;
	OctreeConstPtr zn;
	OctreeConstPtr zp;
	bool isComplete() const { return xn && xp && yn && yp && zn && zp; }
};

void tesselate(OctreePtr octree, NeighbourOctrees const &neighbourOctrees, ChunkGeometryPtr geometry);

class ChunkBuffers
:
	boost::noncopyable
{
	Buffer vertexBuffer;
	Buffer normalBuffer;

	Ranges ranges;

	public:

		Buffer const &getVertexBuffer() const { return vertexBuffer; }
		Buffer &getVertexBuffer() { return vertexBuffer; }
		Buffer const &getNormalBuffer() const { return normalBuffer; }
		Buffer &getNormalBuffer() { return normalBuffer; }
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
			GENERATED,
			TESSELATED,
			LIGHTED
		};

	private:

		int3 const index;
		int3 const position;

		State state;
		bool upgrading;

		OctreePtr octree;
		ChunkGeometryPtr geometry;
		boost::scoped_ptr<ChunkBuffers> buffers;

	public:

		Chunk(int3 const &index);

		int3 const &getIndex() const { return index; }
		int3 const &getPosition() const { return position; }

		State getState() const { return state; }
		bool isUpgrading() const { return upgrading; }

		void startUpgrade();
		void endUpgrade();
		void setOctree(OctreePtr octree);
		void setGeometry(ChunkGeometryPtr geometry);

		OctreePtr getOctree() { return octree; }
		OctreeConstPtr getOctree() const { return octree; }
		ChunkGeometryConstPtr getGeometry() const { return geometry; }

		void render();
	
};

typedef boost::shared_ptr<Chunk> ChunkPtr;
typedef boost::shared_ptr<Chunk const> ChunkConstPtr;

#endif
