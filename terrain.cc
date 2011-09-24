#include "terrain.h"

#include "terragen.h"

size_t CoordsHasher::operator()(int3 const &p) const {
	static std::tr1::hash<float> hasher;
	return hasher(p.x) ^ hasher(p.y) ^ hasher(p.z);
}

Chunk *ChunkMap::get(int3 const &index) const {
	Map::const_iterator i = map.find(index);
	if (i == map.end()) {
		return 0;
	} else {
		return &*i->second;
	}
}

bool ChunkMap::contains(int3 const &index) const {
	return map.find(index) != map.end();
}

void ChunkMap::request(int3 const &index, TerrainGenerator &terrainGenerator) {
	if (!contains(index)) {
		map[index] = Ptr(new Chunk(index, terrainGenerator));
	}
}

Terrain::Terrain(TerrainGenerator *terrainGenerator)
:
	terrainGenerator(terrainGenerator)
{
}

Terrain::~Terrain() {
}

void Terrain::render(Camera const &camera) {
	int3 center = chunkIndexFromPosition(camera.getPosition());
	int radius = 1;
	CoordsBlock coordsBlock(int3(1 + 2 * radius), center - radius);
	for (CoordsBlock::const_iterator i = coordsBlock.begin(); i != coordsBlock.end(); ++i) {
		if (Chunk *chunk = chunkMap.get(*i)) {
			chunk->render();
		} else {
			chunkMap.request(*i, *terrainGenerator);
		}
	}
}

int3 chunkIndexFromPosition(vec3 const &position) {
	return int3(floor(position / (float)CHUNK_SIZE));
}
