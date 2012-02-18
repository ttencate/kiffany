#ifndef CHUNKMANAGER_H
#define CHUNKMANAGER_H

#include "chunk.h"
#include "maths.h"
#include "octree.h"
#include "threadpool.h"

#include <boost/shared_ptr.hpp>
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

class ChunkMap;
class TerrainGenerator;

class ChunkManager {

	template<typename T>
	struct Prioritized {
		float priority; // lower number is higher priority
		T item;
		Prioritized(float priority, T item) : priority(priority), item(item) { }
		bool operator<(Prioritized<T> const &other) const {
			return priority > other.priority;
		}
	};

	typedef Prioritized<int3> PrioritizedIndex;

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

		OctreePtr octreeOrNull(int3 index);
		NeighbourOctrees getNeighbourOctrees(int3 index);

		void enqueueUpgrade(ChunkPtr chunk);
		void enqueueGeneration(ChunkPtr chunk);
		void enqueueTesselation(ChunkPtr chunk);
		void enqueueLighting(ChunkPtr chunk);

		void generate(int3 index);
		void finalizeGeneration(int3 index, OctreePtr octree);
		void tesselate(int3 index, OctreePtr octree, NeighbourOctrees neighbourOctrees);
		void finalizeTesselation(int3 index, ChunkGeometryPtr chunkGeometry);
		void computeLighting(int3 index, ChunkMap const &chunkMap, ChunkGeometryConstPtr chunkGeometry);
		void finalizeLighting(int3 index, ChunkGeometryPtr chunkGeometry);

		template<typename T>
		static Prioritized<T> makePrioritized(float priority, T item) { return Prioritized<T>(priority, item); }

};

#endif
