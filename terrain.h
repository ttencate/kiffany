#ifndef TERRAIN_H
#define TERRAIN_H

#include "camera.h"
#include "chunk.h"
#include "terragen.h"
#include "threadpool.h"

#include <boost/noncopyable.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/unordered_map.hpp>

#include <list>
#include <queue>
#include <utility>

struct CoordsHasher {
	size_t operator()(int3 p) const {
		static boost::hash<float> hasher;
		return hasher(p.x) ^ hasher(p.y) ^ hasher(p.z);
	}
};

class PriorityFunction {
	Camera camera;
	public:
		PriorityFunction() { }
		explicit PriorityFunction(Camera const &camera) : camera(camera) { }
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
	typedef std::pair<float, int3> PriorityPair;
	struct EvictionPriority {
		bool operator()(PriorityPair const &a, PriorityPair const &b) {
			return a.first < b.first;
		}
	};
	typedef std::priority_queue<PriorityPair, std::vector<PriorityPair>, EvictionPriority> EvictionQueue;

	PositionMap map;
	EvictionQueue evictionQueue;
	PriorityFunction priorityFunction;

	public:

		ChunkMap(unsigned maxSize);

		ChunkPtr operator[](int3 index);
		bool contains(int3 index) const;

		void setPriorityFunction(PriorityFunction const &priorityFunction);

	private:

		bool atCapacity() const { return map.size() >= maxSize; }
		bool overCapacity() const { return map.size() > maxSize; }
		void recomputePriorities();
		void trim();

};

class ChunkManager {

	typedef std::pair<float, int3> PriorityPair;
	struct Priority {
		bool operator()(PriorityPair const &a, PriorityPair const &b) {
			return a.first > b.first;
		}
	};
	typedef std::priority_queue<PriorityPair, std::vector<PriorityPair>, Priority> Queue;

	ChunkMap chunkMap;

	boost::scoped_ptr<TerrainGenerator> terrainGenerator;

	PriorityFunction priorityFunction;

	boost::mutex generationQueueMutex;
	Queue generationQueue;
	ThreadPool threadPool;

	public:

		ChunkManager(unsigned maxNumChunks, TerrainGenerator *terrainGenerator);

		ChunkPtr chunkOrNull(int3 index);
		void requestGeneration(ChunkPtr chunk);

		void setPriorityFunction(PriorityFunction const &priorityFunction);
		void gather();

	private:

		ThreadPool::Finalizer generate();
		void finalize(int3 index, ChunkGeometry *chunkGeometry);
		static void doNothing();

};

class Terrain
:
	boost::noncopyable
{

	ChunkManager chunkManager;

	public:

		Terrain(TerrainGenerator *terrainGenerator);
		~Terrain();

		void update(float dt);
		void render(Camera const &camera);

	private:

		unsigned computeMaxNumChunks() const;
		void renderChunk(Camera const &camera, int3 const &index);

};

#endif
