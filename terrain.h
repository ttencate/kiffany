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

struct ViewSphere {
	vec3 center;
	float radius;
	ViewSphere(vec3 center, float radius) : center(center), radius(radius) { }
};

typedef boost::shared_ptr<ViewSphere> ViewSpherePtr;
typedef boost::shared_ptr<ViewSphere const> ConstViewSpherePtr;
typedef boost::weak_ptr<ViewSphere const> WeakConstViewSpherePtr;

class ChunkManager {

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

		ChunkDataPtr chunkDataOrNull(int3 index);
		NeighbourChunkData getNeighbourChunkData(int3 index);

		void requestGeneration(ChunkPtr chunk);
		void requestTesselation(ChunkPtr chunk);
		void requestLighting(ChunkPtr chunk);

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
