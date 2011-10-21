#include "terrain.h"

#include "flags.h"
#include "stats.h"
#include "terragen.h"

float ChunkPriorityFunction::operator()(Chunk const &chunk) const {
	return (*this)(chunk.getIndex());
}

float ChunkPriorityFunction::operator()(int3 index) const {
	return (*this)(chunkCenter(index));
}

float ChunkPriorityFunction::operator()(vec3 chunkCenter) const {
	return 1 / length(chunkCenter - camera.getPosition());
}

ChunkMap::ChunkMap(unsigned maxSize)
:
	maxSize(maxSize)
{
}

ChunkPtr ChunkMap::operator[](int3 index) {
	boost::upgrade_lock<boost::shared_mutex> readLock(mapMutex);
	PositionMap::iterator i = map.find(index);
	if (i == map.end()) {
		float priority = evictionQueue.priority(index);
		if (atCapacity() && (evictionQueue.empty() || priority <= evictionQueue.back_priority())) {
			return ChunkPtr();
		}

		ChunkPtr chunk(new Chunk(index));
		stats.chunksCreated.increment();

		{
			boost::upgrade_to_unique_lock<boost::shared_mutex> writeLock(readLock);

			map[index] = chunk;
			evictionQueue.insert(index);
			trim();
		}

		return chunk;
	}
	return i->second;
}

bool ChunkMap::contains(int3 index) const {
	boost::shared_lock<boost::shared_mutex> readLock(mapMutex);
	return map.find(index) != map.end();
}

void ChunkMap::setPriorityFunction(ChunkPriorityFunction const &priorityFunction) {
	evictionQueue.setPriorityFunction(priorityFunction);
}

void ChunkMap::trim() {
	// Caution: assumes that we hold the write lock!
	while (overCapacity()) {
		stats.chunksEvicted.increment();
		int3 index = evictionQueue.back();
		evictionQueue.pop_back();
		map.erase(index);
	}
}

ChunkManager::ChunkManager(unsigned maxNumChunks, TerrainGenerator *terrainGenerator)
:
	chunkMap(maxNumChunks),
	terrainGenerator(terrainGenerator),
	finalizerQueue(JobThreadPool::defaultNumThreads()),
	threadPool(0)
{
}

ChunkPtr ChunkManager::chunkOrNull(int3 index) {
	return chunkMap[index];
}

void ChunkManager::requestGeneration(ChunkPtr chunk) {
	if (chunk->getState() == Chunk::NEW) {
		int3 index = chunk->getIndex();
		chunk->setGenerating();
		threadPool.enqueue(Job(
					index,
					1.0f,
					boost::bind(
						&ChunkManager::generate, this,
						chunk->getIndex())));
	}
}

void ChunkManager::requestTesselation(ChunkPtr chunk) {
	if (chunk->getState() == Chunk::GENERATED) {
		int3 index = chunk->getIndex();
		NeighbourChunkData neighbourChunkData = getNeighbourChunkData(index);
		if (neighbourChunkData.isComplete()) {
			chunk->setTesselating();
			threadPool.enqueue(Job(
						index,
						1.0e6f,
						boost::bind(
							&ChunkManager::tesselate, this,
							index, chunk->getData(), neighbourChunkData)));
		}
	}
}

void ChunkManager::setPriorityFunction(ChunkPriorityFunction const &priorityFunction) {
	this->chunkPriorityFunction = priorityFunction;
	chunkMap.setPriorityFunction(priorityFunction);
	threadPool.setPriorityFunction(
			JobThreadPool::PriorityFunction(JobPriorityFunction(priorityFunction)));
}

void ChunkManager::gather() {
	finalizerQueue.runAll();
	stats.irrelevantJobsSkipped.increment(
			threadPool.cleanUp(boost::bind(&ChunkManager::isJobIrrelevant, this, _1)));
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

bool ChunkManager::isJobIrrelevant(Job const &job) {
	return !chunkMap.contains(job.index);
}

void ChunkManager::generate(int3 index) {
	int3 position = chunkPositionFromIndex(index);

	RawChunkData rawChunkData;
	terrainGenerator->generateChunk(position, rawChunkData);

	ChunkDataPtr chunkData(new ChunkData());
	compress(rawChunkData, *chunkData);

	OctreePtr octree(new Octree());
	buildOctree(rawChunkData, *octree);

	finalizerQueue.post(boost::bind(
				&ChunkManager::finalizeGeneration, this, index, chunkData, octree));
}

void ChunkManager::finalizeGeneration(int3 index, ChunkDataPtr chunkData, OctreePtr octree) {
	ChunkPtr chunk = chunkOrNull(index);
	if (!chunk) {
		stats.irrelevantJobsRun.increment();
		return;
	}
	chunk->setData(chunkData);
	chunk->setOctree(octree);
}

void ChunkManager::tesselate(int3 index, ChunkDataPtr chunkData, NeighbourChunkData neighbourChunkData) {
	ChunkGeometryPtr chunkGeometry(new ChunkGeometry());
	::tesselate(chunkData, neighbourChunkData, chunkGeometry);

	finalizerQueue.post(boost::bind(
				&ChunkManager::finalizeTesselation, this, index, chunkGeometry));
}

void ChunkManager::finalizeTesselation(int3 index, ChunkGeometryPtr chunkGeometry) {
	ChunkPtr chunk = chunkOrNull(index);
	if (!chunk) {
		stats.irrelevantJobsRun.increment();
		return;
	}
	chunk->setGeometry(chunkGeometry);
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
	chunkManager.setPriorityFunction(ChunkPriorityFunction(camera));

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
