#ifndef WORLD_H
#define WORLD_H

#include "chunk.h"

#include <boost/shared_ptr.hpp>

#include <tr1/unordered_map>

struct CoordsHasher {
	size_t operator()(glm::int3 const &p) const;
};

class World {

	typedef boost::shared_ptr<Chunk> ChunkPtr;
	typedef std::tr1::unordered_map<glm::int3, ChunkPtr, CoordsHasher> ChunkMap;

	ChunkMap chunks;

	public:

		World();

		void render() const;

};

#endif
