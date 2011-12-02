#ifndef TERRAIN_H
#define TERRAIN_H

#include "chunkmap.h"
#include "maths.h"
#include "octree.h"
#include "terragen.h"
#include "threadpool.h"

#include <boost/noncopyable.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/weak_ptr.hpp>

#include <queue>

struct ViewSphere {
	vec3 center;
	float radius;
	ViewSphere(vec3 center, float radius) : center(center), radius(radius) { }
};

typedef boost::shared_ptr<ViewSphere> ViewSpherePtr;
typedef boost::shared_ptr<ViewSphere const> ConstViewSpherePtr;
typedef boost::weak_ptr<ViewSphere const> WeakConstViewSpherePtr;

template<typename T>
struct Prioritized {
	float priority; // lower number is higher priority
	T item;
	Prioritized(float priority, T item) : priority(priority), item(item) { }
	bool operator<(Prioritized<T> const &other) const {
		return priority > other.priority;
	}
};

template<typename T>
Prioritized<T> makePrioritized(float priority, T item) { return Prioritized<T>(priority, item); }

typedef Prioritized<int3> PrioritizedIndex;

class ChunkManager {

	typedef std::priority_queue<PrioritizedIndex> PriorityQueue;

	ChunkMap &chunkMap;

	boost::scoped_ptr<TerrainGenerator> terrainGenerator;

	std::vector<WeakConstViewSpherePtr> viewSpheres;

	WorkQueue finalizerQueue;
	ThreadPool threadPool;

	public:

		ChunkManager(ChunkMap &chunkMap, TerrainGenerator *terrainGenerator);

		void addViewSphere(WeakConstViewSpherePtr viewSphere);

		void sow();
		void reap();

	private:

		bool tryUpgradeChunk(PrioritizedIndex prioIndex, PriorityQueue &queue);

		ChunkDataPtr chunkDataOrNull(int3 index);
		NeighbourChunkData getNeighbourChunkData(int3 index);

		void enqueueUpgrade(ChunkPtr chunk);
		void enqueueGeneration(ChunkPtr chunk);
		void enqueueTesselation(ChunkPtr chunk);
		void enqueueLighting(ChunkPtr chunk);

		void generate(int3 index);
		void finalizeGeneration(int3 index, ChunkDataPtr chunkData, OctreePtr octree);
		void tesselate(int3 index, ChunkDataPtr chunkData, NeighbourChunkData neighbourChunkData);
		void finalizeTesselation(int3 index, ChunkGeometryPtr chunkGeometry);
		void computeLighting(int3 index, ChunkMap const &chunkMap, ChunkGeometryConstPtr chunkGeometry);
		void finalizeLighting(int3 index, ChunkGeometryPtr chunkGeometry);

};

class Terrain
:
	boost::noncopyable
{

	ChunkMap chunkMap;
	ChunkManager chunkManager;

	public:

		Terrain(TerrainGenerator *terrainGenerator);
		~Terrain();

		ChunkMap const &getChunkMap() const { return chunkMap; }

		void addViewSphere(WeakConstViewSpherePtr viewSphere);

		void update(float dt);
		void render(Camera const &camera);

	private:

		unsigned computeMaxNumChunks() const;
		void renderChunk(Camera const &camera, int3 const &index);

};

#endif
