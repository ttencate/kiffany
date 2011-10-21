#ifndef TERRAIN_H
#define TERRAIN_H

#include "chunkmap.h"
#include "octree.h"
#include "terragen.h"
#include "threadpool.h"

#include <boost/noncopyable.hpp>
#include <boost/scoped_ptr.hpp>

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
		void finalizeGeneration(int3 index, ChunkDataPtr chunkData, OctreePtr octree);
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
