#include "terrain.h"

#include "flags.h"
#include "stats.h"
#include "terragen.h"

float PriorityFunction::operator()(Chunk const &chunk) const {
	return length(chunkCenter(chunk.getIndex()) - camera.getPosition());
}

ChunkMap::ChunkMap(unsigned maxSize)
:
	maxSize(maxSize)
{
}

ChunkPtr ChunkMap::operator[](int3 index) {
	PositionMap::iterator i = map.find(index);
	if (i == map.end()) {
		ChunkPtr chunk(new Chunk(index));
		float priority = priorityFunction(*chunk);

		evictionQueue.push(PriorityPair(priority, index));
		stats.chunksCreated.increment();

		map[index] = chunk;
		trim(); // Note: this might evict the chunk we just created!

		return chunk;
	}
	return i->second;
}

bool ChunkMap::contains(int3 index) const {
	return map.find(index) != map.end();
}

void ChunkMap::setPriorityFunction(PriorityFunction const &priorityFunction) {
	this->priorityFunction = priorityFunction;
	recomputePriorities();
}

void ChunkMap::trim() {
	while (atCapacity()) {
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

ChunkManager::ChunkManager(unsigned maxNumChunks, TerrainGenerator *terrainGenerator)
:
	chunkMap(maxNumChunks),
	terrainGenerator(terrainGenerator),
	threadPool(0, ThreadPool::defaultNumThreads())
{
}

ChunkPtr ChunkManager::chunkAtIndex(int3 index) {
	return chunkMap[index];
}

void ChunkManager::requestGeneration(int3 index) {
	ChunkPtr chunk = chunkMap[index];
	if (chunk->needsGenerating()) {
		{
			boost::unique_lock<boost::mutex> lock(generationQueueMutex);
			generationQueue.push(PriorityPair(priorityFunction(*chunk), index));
		}
		chunk->generating();
		threadPool.enqueue(boost::bind(&ChunkManager::generate, this));
	}
}

void ChunkManager::setPriorityFunction(PriorityFunction const &priorityFunction) {
	this->priorityFunction = priorityFunction;
	chunkMap.setPriorityFunction(priorityFunction);
}

void ChunkManager::gather() {
	threadPool.runFinalizers();
}

ThreadPool::Finalizer ChunkManager::generate() {
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

	return boost::bind(&ChunkManager::finalize, this, index, chunkGeometry);
}

void ChunkManager::finalize(int3 index, ChunkGeometry *geometry) {
	if (chunkMap.contains(index)) {
		ChunkPtr chunk = chunkMap[index];
		chunk->setGeometry(geometry);
	} else {
		// The chunk has been evicted in the meantime. Tough luck.
		delete geometry;
	}
}

void ChunkManager::doNothing() {
}

Terrain::Terrain(TerrainGenerator *terrainGenerator)
:
	chunkManager(computeMaxNumChunks(), terrainGenerator)
{
}

Terrain::~Terrain() {
}

void Terrain::update(float dt) {
	chunkManager.gather();
}

void Terrain::render(Camera const &camera) {
	chunkManager.setPriorityFunction(PriorityFunction(camera));

	int3 center = chunkIndexFromPosition(camera.getPosition());
	int radius = flags.viewDistance / CHUNK_SIZE;
	for (int z = -radius; z <= radius; ++z) {
		for (int y = -radius; y <= radius; ++y) {
			for (int x = -radius; x <= radius; ++x) {
				renderChunk(camera, center + int3(x, y, z));
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
	ChunkPtr chunk = chunkManager.chunkAtIndex(index);
	chunkManager.requestGeneration(index);
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
