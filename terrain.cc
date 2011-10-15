#include "terrain.h"

#include "flags.h"
#include "stats.h"
#include "terragen.h"

float PriorityFunction::operator()(Chunk const &chunk) const {
	return (*this)(chunk.getIndex());
}

float PriorityFunction::operator()(int3 index) const {
	return (*this)(chunkCenter(index));
}

float PriorityFunction::operator()(vec3 chunkCenter) const {
	return -length(chunkCenter - camera.getPosition());
}

ChunkMap::ChunkMap(unsigned maxSize)
:
	maxSize(maxSize)
{
}

ChunkPtr ChunkMap::operator[](int3 index) {
	PositionMap::iterator i = map.find(index);
	if (i == map.end()) {
		float priority = priorityFunction(index);
		if (atCapacity() && (evictionQueue.empty() || priority <= evictionQueue.top().first)) {
			return ChunkPtr();
		}

		ChunkPtr chunk(new Chunk(index));
		stats.chunksCreated.increment();

		map[index] = chunk;
		evictionQueue.push(PriorityPair(priority, index));
		trim();

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
	while (overCapacity()) {
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
		float priority = priorityFunction(index);
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

ChunkPtr ChunkManager::chunkOrNull(int3 index) {
	return chunkMap[index];
}

void ChunkManager::requestGeneration(ChunkPtr chunk) {
	if (chunk->getState() == Chunk::NEW) {
		float priority = priorityFunction(*chunk);
		chunk->setGenerating();
		threadPool.enqueue(
				boost::bind(
					&ChunkManager::generate, this,
					chunk->getIndex()),
				priority);
	}
}

void ChunkManager::requestTesselation(ChunkPtr chunk) {
	if (chunk->getState() == Chunk::GENERATED) {
		int3 index = chunk->getIndex();
		NeighbourChunkData neighbourChunkData = getNeighbourChunkData(index);
		if (neighbourChunkData.isComplete()) {
			float priority = priorityFunction(*chunk);
			chunk->setTesselating();
			threadPool.enqueue(
					boost::bind(
						&ChunkManager::tesselate, this,
						index, chunk->getData(), neighbourChunkData),
					priority);
		}
	}
}

void ChunkManager::setPriorityFunction(PriorityFunction const &priorityFunction) {
	this->priorityFunction = priorityFunction;
	chunkMap.setPriorityFunction(priorityFunction);
}

void ChunkManager::gather() {
	threadPool.runFinalizers();
}

ChunkDataPtr ChunkManager::chunkDataOrNull(int3 index) {
	ChunkPtr chunk = chunkOrNull(index);
	if (!chunk) {
		return ChunkDataPtr();
	}
	requestGeneration(chunk);
	return chunk->getData();
}

NeighbourChunkData ChunkManager::getNeighbourChunkData(int3 index) {
	NeighbourChunkData data;
	data.xn = chunkDataOrNull(index + int3(-1,  0,  0));
	data.xp = chunkDataOrNull(index + int3( 1,  0,  0));
	data.yn = chunkDataOrNull(index + int3( 0, -1,  0));
	data.yp = chunkDataOrNull(index + int3( 0,  1,  0));
	data.zn = chunkDataOrNull(index + int3( 0,  0, -1));
	data.zp = chunkDataOrNull(index + int3( 0,  0,  1));
	return data;
}

ThreadPool::Finalizer ChunkManager::generate(int3 index) {
	int3 position = chunkPositionFromIndex(index);

	ChunkDataPtr chunkData(new ChunkData());
	terrainGenerator->generateChunk(position, chunkData);

	return boost::bind(&ChunkManager::finalizeGeneration, this, index, chunkData);
}

void ChunkManager::finalizeGeneration(int3 index, ChunkDataPtr chunkData) {
	ChunkPtr chunk = chunkOrNull(index);
	if (chunk) {
		chunk->setData(chunkData);
	}
}

ThreadPool::Finalizer ChunkManager::tesselate(int3 index, ChunkDataPtr chunkData, NeighbourChunkData neighbourChunkData) {
	int3 position = chunkPositionFromIndex(index);

	ChunkGeometryPtr chunkGeometry(new ChunkGeometry());
	::tesselate(chunkData, neighbourChunkData, position, chunkGeometry);

	return boost::bind(&ChunkManager::finalizeTesselation, this, index, chunkGeometry);
}

void ChunkManager::finalizeTesselation(int3 index, ChunkGeometryPtr chunkGeometry) {
	ChunkPtr chunk = chunkOrNull(index);
	if (chunk) {
		chunk->setGeometry(chunkGeometry);
	}
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
	ChunkPtr chunk = chunkManager.chunkOrNull(index);
	if (!chunk) {
		return;
	}
	chunkManager.requestGeneration(chunk);
	chunkManager.requestTesselation(chunk);
	stats.chunksConsidered.increment();
	if (chunk->getState() < Chunk::TESSELATED) {
		stats.chunksSkipped.increment();
	} else {
		if (camera.isSphereInView(chunkCenter(index), CHUNK_RADIUS)) {
			chunk->render();
		} else {
			stats.chunksCulled.increment();
		}
	}
}
