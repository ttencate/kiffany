#ifndef TERRAIN_H
#define TERRAIN_H

#include "chunk.h"

#include <boost/shared_ptr.hpp>

#include <tr1/unordered_map>

struct CoordsHasher {
	size_t operator()(glm::int3 const &p) const;
};

class Terrain {

	typedef boost::shared_ptr<Chunk> ChunkPtr;
	typedef std::tr1::unordered_map<glm::int3, ChunkPtr, CoordsHasher> ChunkMap;

	ChunkMap chunks;

	public:

		Terrain();
		~Terrain();

		void render() const;

};

#endif
