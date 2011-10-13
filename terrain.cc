#include "terrain.h"

#include "flags.h"
#include "stats.h"
#include "terragen.h"

size_t CoordsHasher::operator()(int3 const &p) const {
	static boost::hash<float> hasher;
	return hasher(p.x) ^ hasher(p.y) ^ hasher(p.z);
}

float PriorityFunction::operator()(Chunk const &chunk) const {
	return length(chunkCenter(chunk.getIndex()) - camera.getPosition());
}

ChunkPtr ChunkMap::get(int3 const &index) {
	ChunkPtr &chunkPtr = map[index];
	if (!chunkPtr) {
		chunkPtr.reset(new Chunk(index));
	}
	return chunkPtr;
}

void ChunkMap::setPriorityFunction(PriorityFunction const &priorityFunction) {
	this->priorityFunction = priorityFunction;
	recomputePriorities();
}

void ChunkMap::trimToSize(unsigned size) {
	while (map.size() > size) {
		stats.chunksEvicted.increment();
		int3 index = queue.top().second;
		map.erase(index);
		queue.pop();
	}
}

void ChunkMap::recomputePriorities() {
	queue = PriorityQueue(); // it has no clear() function
	for (PositionMap::const_iterator i = map.begin(); i != map.end(); ++i) {
		int3 const &index = i->first;
		Chunk const &chunk = *i->second;
		float priority = priorityFunction(chunk);
		queue.push(std::make_pair(priority, index));
	}
}

Terrain::Terrain(TerrainGenerator *terrainGenerator)
:
	terrainGenerator(terrainGenerator),
	asyncTerrainGenerator(*terrainGenerator),
	maxNumChunks(computeMaxNumChunks())
{
}

Terrain::~Terrain() {
}

void Terrain::update(float dt) {
	asyncTerrainGenerator.gather();
}

void Terrain::render(Camera const &camera) {
	chunkMap.setPriorityFunction(PriorityFunction(camera));

	int3 center = chunkIndexFromPosition(camera.getPosition());
	int radius = flags.viewDistance / CHUNK_SIZE;
	for (int z = -radius; z <= radius; ++z) {
		for (int y = -radius; y <= radius; ++y) {
			for (int x = -radius; x <= radius; ++x) {
				renderChunk(camera, center + int3(x, y, z));
			}
		}
	}

	chunkMap.trimToSize(maxNumChunks);
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
