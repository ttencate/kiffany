#ifndef CHUNKMAP_H
#define CHUNKMAP_H

#include "camera.h"
#include "chunk.h"
#include "maths.h"

#include <boost/noncopyable.hpp>
#include <boost/thread.hpp>
#include <boost/unordered_map.hpp>

struct CoordsHasher {
	size_t operator()(int3 p) const {
		static boost::hash<float> hasher;
		return hasher(p.x) ^ hasher(p.y) ^ hasher(p.z);
	}
};

class ChunkMap
:
	boost::noncopyable
{

	typedef boost::unordered_map<int3, ChunkPtr, CoordsHasher> PositionMap;

	boost::shared_mutex mutable mapMutex;
	PositionMap map;

	public:

		ChunkMap();

		ChunkPtr operator[](int3 index);
		ChunkConstPtr operator[](int3 index) const;
		bool contains(int3 index) const;
		Chunk::State getChunkState(int3 index) const;
		bool isChunkUpgrading(int3 index) const;

};

#endif
