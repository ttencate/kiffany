#ifndef TERRAIN_H
#define TERRAIN_H

#include "camera.h"
#include "chunk.h"
#include "terragen.h"

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
	struct PriorityPairCompare {
		bool operator()(PriorityPair const &a, PriorityPair const &b) {
			return a.first < b.first;
		}
	};
	typedef std::priority_queue<PriorityPair, std::vector<PriorityPair>, PriorityPairCompare > PriorityQueue;

	PositionMap map;
	PriorityQueue queue;
	PriorityFunction priorityFunction;

	public:

		ChunkPtr get(int3 const &index);

		void setPriorityFunction(PriorityFunction const &priorityFunction);
		void trimToSize(unsigned size);

	private:

		void recomputePriorities();

};

class Terrain
:
	boost::noncopyable
{

	boost::scoped_ptr<TerrainGenerator> terrainGenerator;
	AsyncTerrainGenerator asyncTerrainGenerator;

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
