#ifndef CHUNK_H
#define CHUNK_H

#include "buffer.h"
#include "chunkdata.h"
#include "geometry.h"
#include "octree.h"
#include "gl.h"

#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>

#include <utility>

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
