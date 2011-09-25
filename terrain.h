#ifndef TERRAIN_H
#define TERRAIN_H

#include "camera.h"
#include "chunk.h"
#include "terragen.h"

#include <boost/noncopyable.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>

#include <tr1/unordered_map>

struct CoordsHasher {
	size_t operator()(int3 const &p) const;
};

class ChunkMap
:
	boost::noncopyable
{

	typedef boost::shared_ptr<Chunk> Ptr;
	typedef std::tr1::unordered_map<int3, Ptr, CoordsHasher> Map;

	Map map;

	public:

		Chunk *get(int3 const &index) const;
		bool contains(int3 const &index) const;
		void put(int3 const &index, Ptr chunk);

};

class Terrain
:
	boost::noncopyable
{

	boost::scoped_ptr<TerrainGenerator> terrainGenerator;
	AsyncTerrainGenerator asyncTerrainGenerator;

	ChunkMap chunkMap;

	public:

		Terrain(TerrainGenerator *terrainGenerator);
		~Terrain();

		void render(Camera const &camera);

};

int3 chunkIndexFromPosition(vec3 const &position);

#endif
