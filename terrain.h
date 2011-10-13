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
	size_t operator()(int3 const &p) const;
};

class PriorityFunction {
	Camera camera;
	public:
		PriorityFunction() { }
		explicit PriorityFunction(Camera const &camera) : camera(camera) { }
		float operator()(Chunk const &chunk) const;
};

class ChunkMap
:
	boost::noncopyable
{

	typedef boost::unordered_map<int3, ChunkPtr, CoordsHasher> PositionMap;
	typedef std::pair<float, int3> PriorityPair;
	struct EvictionPriority {
		bool operator()(PriorityPair const &a, PriorityPair const &b) {
			return a.first < b.first;
		}
	};
	struct GenerationPriority {
		bool operator()(PriorityPair const &a, PriorityPair const &b) {
			return a.first > b.first;
		}
	};
	typedef std::priority_queue<PriorityPair, std::vector<PriorityPair>, EvictionPriority> EvictionQueue;
	typedef std::priority_queue<PriorityPair, std::vector<PriorityPair>, GenerationPriority> GenerationQueue;

	PositionMap map;
	EvictionQueue evictionQueue;
	PriorityFunction priorityFunction;

	boost::scoped_ptr<TerrainGenerator> terrainGenerator;

	boost::mutex generationQueueMutex;
	GenerationQueue generationQueue;
	ThreadPool threadPool;

	public:

		ChunkMap(TerrainGenerator *terrainGenerator);

		ChunkPtr get(int3 const &index);

		void gather();
		void setPriorityFunction(PriorityFunction const &priorityFunction);
		void trimToSize(unsigned size);

	private:

		void recomputePriorities();
		ThreadPool::Finalizer tryGenerateOne();
		void finalize(int3 index, ChunkGeometry *chunkGeometry);
		static void doNothing();

};

class Terrain
:
	boost::noncopyable
{

	ChunkMap chunkMap;

	unsigned const maxNumChunks;

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
