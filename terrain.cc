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

ChunkMap::ChunkMap(TerrainGenerator *terrainGenerator)
:
	terrainGenerator(terrainGenerator),
	threadPool(0, 0)
{
}

ChunkPtr ChunkMap::get(int3 const &index) {
	ChunkPtr &chunk = map[index];
	if (!chunk) {
		chunk.reset(new Chunk(index));
		evictionQueue.push(PriorityPair(priorityFunction(*chunk), index));
		{
			boost::unique_lock<boost::mutex> lock(generationQueueMutex);
			generationQueue.push(PriorityPair(priorityFunction(*chunk), index));
		}
		threadPool.enqueue(boost::bind(&ChunkMap::tryGenerateOne, this));
	}
	return chunk;
}

void ChunkMap::gather() {
	threadPool.runFinalizers();
}

void ChunkMap::setPriorityFunction(PriorityFunction const &priorityFunction) {
	this->priorityFunction = priorityFunction;
	recomputePriorities();
}

void ChunkMap::trimToSize(unsigned size) {
	while (map.size() > size) {
		stats.chunksEvicted.increment();
		int3 index = evictionQueue.top().second;
		map.erase(index);
		evictionQueue.pop();
	}
}

void ChunkMap::recomputePriorities() {
	evictionQueue = EvictionQueue(); // it has no clear() function
	for (PositionMap::const_iterator i = map.begin(); i != map.end(); ++i) {
		int3 const &index = i->first;
		Chunk const &chunk = *i->second;
		float priority = priorityFunction(chunk);
		evictionQueue.push(PriorityPair(priority, index));
	}
}

ThreadPool::Finalizer ChunkMap::tryGenerateOne() {
	int3 index;
	{
		boost::unique_lock<boost::mutex> lock(generationQueueMutex);
		if (generationQueue.empty()) {
			return boost::bind(&doNothing);
		}
		index = generationQueue.top().second;
		generationQueue.pop();
	}

	int3 position = chunkPositionFromIndex(index);

	boost::scoped_ptr<ChunkData> chunkData(new ChunkData());
	terrainGenerator->generateChunk(position, chunkData.get());

	ChunkGeometry *chunkGeometry = new ChunkGeometry();
	tesselate(*chunkData, position, chunkGeometry);

	return boost::bind(&ChunkMap::finalize, this, index, chunkGeometry);
}

void ChunkMap::finalize(int3 index, ChunkGeometry *geometry) {
	if (map.find(index) != map.end()) {
		ChunkPtr chunk = map[index];
		chunk->setGeometry(geometry);
	} else {
		// The chunk has been evicted in the meantime.
		delete geometry;
	}
}

void ChunkMap::doNothing() {
}

Terrain::Terrain(TerrainGenerator *terrainGenerator)
:
	chunkMap(terrainGenerator),
	maxNumChunks(computeMaxNumChunks())
{
}

Terrain::~Terrain() {
}

void Terrain::update(float dt) {
	chunkMap.gather();
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
	stats.chunksConsidered.increment();
	if (!chunk->readyForRendering()) {
		stats.chunksSkipped.increment();
	} else {
		if (camera.isSphereInView(chunkCenter(index), CHUNK_RADIUS)) {
			chunk->render();
		} else {
			stats.chunksCulled.increment();
		}
	}
}
