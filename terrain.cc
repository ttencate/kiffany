#include "terrain.h"

#include "flags.h"
#include "stats.h"
#include "terragen.h"

size_t CoordsHasher::operator()(int3 const &p) const {
	static boost::hash<float> hasher;
	return hasher(p.x) ^ hasher(p.y) ^ hasher(p.z);
}

ChunkMap::ChunkMap(unsigned maxSize)
:
	maxSize(maxSize)
{
}

ChunkPtr ChunkMap::get(int3 const &index) {
	ChunkPtr &chunkPtr = map[index];
	if (!chunkPtr) {
		chunkPtr.reset(new Chunk(index));
	}
	trimToSize();
	return chunkPtr;
}

void ChunkMap::trimToSize() {
	if (maxSize > 0) {
		while (map.size() > maxSize) {
			stats.chunksEvicted.increment();
			map.pop_back();
		}
	}
}

Terrain::Terrain(TerrainGenerator *terrainGenerator)
:
	terrainGenerator(terrainGenerator),
	asyncTerrainGenerator(*terrainGenerator),
	chunkMap(computeMaxNumChunks())
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

unsigned Terrain::computeMaxNumChunks() const {
	if (flags.maxNumChunks != 0) {
		return flags.maxNumChunks;
	}
	unsigned radius = flags.viewDistance / CHUNK_SIZE;
	unsigned size = 2 * radius + 1;
	return size * size * size;
}

void Terrain::renderChunk(Camera const &camera, int3 const &index) {
	ChunkPtr chunk = chunkMap.get(index);
	if (chunk->needsGenerating()) {
		asyncTerrainGenerator.tryGenerate(chunk);
	} else {
		if (camera.isSphereInView(chunkCenter(index), CHUNK_RADIUS)) {
			chunk->render();
		} else {
			stats.chunksCulled.increment();
		}
	}
}
