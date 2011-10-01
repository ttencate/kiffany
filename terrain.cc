#include "terrain.h"

#include "flags.h"
#include "stats.h"
#include "terragen.h"

size_t CoordsHasher::operator()(int3 const &p) const {
	static boost::hash<float> hasher;
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
	// Render in roughly front-to-back order to make chunks generate in that order
	for (int r = 0; r <= radius; ++r) {
		for (int z = -r; z <= r; ++z) {
			for (int y = -r; y <= r; ++y) {
				renderChunk(camera, center + int3(-r, y, z));
				renderChunk(camera, center + int3(r, y, z));
			}
		}
		for (int z = -r; z <= r; ++z) {
			for (int x = -r + 1; x <= r - 1; ++x) {
				renderChunk(camera, center + int3(x, -r, z));
				renderChunk(camera, center + int3(x, r, z));
			}
		}
		for (int y = -r + 1; y <= r - 1; ++y) {
			for (int x = -r + 1; x <= r - 1; ++x) {
				renderChunk(camera, center + int3(x, y, -r));
				renderChunk(camera, center + int3(x, y, r));
			}
		}
	}
}

void Terrain::renderChunk(Camera const &camera, int3 const &index) {
	if (Chunk *chunk = chunkMap.get(index)) {
		if (chunk->canRender()) {
		   if (camera.isSphereInView(chunkCenter(index), CHUNK_RADIUS)) {
			   chunk->render();
		   } else {
			   stats.chunksCulled.increment();
		   }
		}
	} else {
		boost::shared_ptr<Chunk> newChunk(new Chunk(index));
		chunkMap.put(index, newChunk);
		asyncTerrainGenerator.generate(newChunk.get());
	}
}
