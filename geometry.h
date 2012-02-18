#ifndef GEOMETRY_H
#define GEOMETRY_H

#include "octree.h"

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

#endif
