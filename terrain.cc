#include "terrain.h"

#include "flags.h"
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

void ChunkMap::put(int3 const &index, Ptr chunk) {
	map[index] = chunk;
}

Terrain::Terrain(TerrainGenerator *terrainGenerator)
:
	terrainGenerator(terrainGenerator),
	asyncTerrainGenerator(*terrainGenerator)
{
}

Terrain::~Terrain() {
}

void Terrain::update(float dt) {
	asyncTerrainGenerator.gather();
}

void Terrain::render(Camera const &camera) {
	int3 center = chunkIndexFromPosition(camera.getPosition());
	int radius = flags.viewDistance / CHUNK_SIZE;
	CoordsBlock coordsBlock(int3(1 + 2 * radius), center - radius);
	for (CoordsBlock::const_iterator i = coordsBlock.begin(); i != coordsBlock.end(); ++i) {
		if (Chunk *chunk = chunkMap.get(*i)) {
			if (chunk->isGenerated()) {
				chunk->render();
			}
		} else {
			boost::shared_ptr<Chunk> newChunk(new Chunk(*i));
			chunkMap.put(*i, newChunk);
			asyncTerrainGenerator.generate(newChunk.get());
		}
	}
}

int3 chunkIndexFromPosition(vec3 const &position) {
	return int3(floor(position / (float)CHUNK_SIZE));
}
