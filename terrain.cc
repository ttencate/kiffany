#include "terrain.h"

size_t CoordsHasher::operator()(int3 const &p) const {
	static std::tr1::hash<float> hasher;
	return hasher(p.x) ^ hasher(p.y) ^ hasher(p.z);
}

Terrain::Terrain() {
	chunks[int3(0, 0, 0)] = ChunkPtr(new Chunk(int3(0, 0, 0)));
	chunks[int3(1, 1, 1)] = ChunkPtr(new Chunk(int3(1, 1, 1)));
}

Terrain::~Terrain() {
}

void Terrain::render() const {
	for (ChunkMap::const_iterator i = chunks.begin(); i != chunks.end(); ++i) {
		i->second->render();
	}
}
