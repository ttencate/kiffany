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

class ChunkPriorityFunction {
	Camera camera;
	public:
		ChunkPriorityFunction() { }
		explicit ChunkPriorityFunction(Camera const &camera) : camera(camera) { }
		float operator()(Chunk const &chunk) const;
		float operator()(int3 index) const;
		float operator()(vec3 chunkCenter) const;
};

class ChunkMap
:
	boost::noncopyable
{

	unsigned const maxSize;

	typedef boost::unordered_map<int3, ChunkPtr, CoordsHasher> PositionMap;

	boost::shared_mutex mutable mapMutex;
	PositionMap map;

	public:

		ChunkMap(unsigned maxSize);

		ChunkPtr operator[](int3 index);
		ChunkConstPtr operator[](int3 index) const;
		bool contains(int3 index) const;
		Chunk::State getChunkState(int3 index) const;

	private:

		bool atCapacity() const { return map.size() >= maxSize; }
		bool overCapacity() const { return map.size() > maxSize; }
		void trim();

};

#endif
