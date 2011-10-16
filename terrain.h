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
	typedef DynamicPriorityQueue<int3> EvictionQueue;

	PositionMap map;
	EvictionQueue evictionQueue;

	public:

		ChunkMap(unsigned maxSize);

		ChunkPtr operator[](int3 index);
		bool contains(int3 index) const;

		void setPriorityFunction(ChunkPriorityFunction const &priorityFunction);

	private:

		bool atCapacity() const { return map.size() >= maxSize; }
		bool overCapacity() const { return map.size() > maxSize; }
		void trim();

};

class ChunkManager {

	struct Job {

		typedef boost::function<void(void)> Worker;

		int3 const index;
		float const multiplier;
		Worker const worker;

		Job(int3 index, float multiplier, Worker const &worker) : index(index), multiplier(multiplier), worker(worker) { }

		void operator()() const {
			worker();
		}

	};

	struct JobPriorityFunction {
		ChunkPriorityFunction const chunkPriorityFunction;
		JobPriorityFunction(ChunkPriorityFunction const &chunkPriorityFunction)
		:
			chunkPriorityFunction(chunkPriorityFunction)
		{
		}
		float operator()(Job const &job) const {
			return chunkPriorityFunction(job.index) * job.multiplier;
		}
	};

	typedef ThreadPool<Job> JobThreadPool;
	typedef boost::function<void(void)> Finalizer;
	typedef WorkQueue<Finalizer> FinalizerQueue;

	ChunkMap chunkMap;

	boost::scoped_ptr<TerrainGenerator> terrainGenerator;

	ChunkPriorityFunction chunkPriorityFunction;

	FinalizerQueue finalizerQueue;
	JobThreadPool threadPool;

	public:

		ChunkManager(unsigned maxNumChunks, TerrainGenerator *terrainGenerator);

		ChunkPtr chunkOrNull(int3 index);
		void requestGeneration(ChunkPtr chunk);
		void requestTesselation(ChunkPtr chunk);

		void setPriorityFunction(ChunkPriorityFunction const &priorityFunction);
		void gather();

	private:

		ChunkDataPtr chunkDataOrNull(int3 index);
		NeighbourChunkData getNeighbourChunkData(int3 index);

		bool isJobIrrelevant(Job const &job);

		void generate(int3 index);
		void finalizeGeneration(int3 index, ChunkDataPtr chunkData);
		void tesselate(int3 index, ChunkDataPtr chunkData, NeighbourChunkData neighbourChunkData);
		void finalizeTesselation(int3 index, ChunkGeometryPtr chunkGeometry);

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
