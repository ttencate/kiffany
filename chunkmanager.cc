#include "chunkmanager.h"

#include "chunkmap.h"
#include "stats.h"
#include "terragen.h"

ChunkManager::ChunkManager(ChunkMap &chunkMap, TerrainGenerator *terrainGenerator)
:
	chunkMap(chunkMap),
	terrainGenerator(terrainGenerator),
	finalizerQueue(0), // infinite, otherwise threads may block at program exit
	threadPool(ThreadPool::defaultNumThreads())
{
}

void ChunkManager::addViewSphere(WeakConstViewSpherePtr viewSphere) {
	viewSpheres.push_back(viewSphere);
}

void ChunkManager::sow() {
	unsigned jobsToAdd = threadPool.getMaxQueueSize() - threadPool.getQueueSize();
	if (jobsToAdd == 0) {
		return;
	}

	PriorityQueue queue;
	for (unsigned i = 0; i < viewSpheres.size(); ++i) {
		if (ConstViewSpherePtr sphere = viewSpheres[i].lock()) {
			// TODO write chunksInSphere iterator
			int3 min = chunkIndexFromPoint(sphere->center - sphere->radius);
			int3 max = chunkIndexFromPoint(sphere->center + sphere->radius);
			for (int z = min.z; z <= max.z; ++z) {
				for (int y = min.y; y <= max.y; ++y) {
					for (int x = min.x; x <= max.x; ++x) {
						// TODO filter on the sphere
						int3 index(x, y, z);
						float priority = length(chunkCenter(index) - sphere->center); // lower is better
						queue.push(makePrioritized(priority, index));
					}
				}
			}
		} else {
			viewSpheres.erase(viewSpheres.begin() + i);
		}
	}

	while (jobsToAdd && !queue.empty()) {
		PrioritizedIndex prioIndex = queue.top();
		queue.pop();
		if (tryUpgradeChunk(prioIndex, queue)) {
			--jobsToAdd;
		}
	}
}

void ChunkManager::reap() {
	finalizerQueue.runAll();
}

bool ChunkManager::tryUpgradeChunk(PrioritizedIndex prioIndex, PriorityQueue &queue) {
	int3 const index = prioIndex.item;
	if (chunkMap.isChunkUpgrading(index)) {
		return 0;
	}

	Chunk::State const state = chunkMap.getChunkState(index);
	// TODO replace by helper function
	if (state >= Chunk::TESSELATED) {
		return 0;
	}

	Chunk::State const nextState = (Chunk::State)(state + 1);
	int radius;
	// TODO replace by helper function
	switch (nextState) {
		case Chunk::GENERATED: radius = 0; break;
		case Chunk::TESSELATED: radius = 1; break; // TODO get from lighting radius
		default: BOOST_ASSERT(false);
	}

	Chunk::State const minNeighState = state;
	int3 const min = index - radius;
	int3 const max = index + radius;
	bool upgradeSelf = true;
	// TODO write chunksInCube iterator
	for (int z = min.z; z <= max.z; ++z) {
		for (int y = min.y; y <= max.y; ++y) {
			for (int x = min.x; x <= max.x; ++x) {
				int3 const neighIndex(x, y, z);
				if (neighIndex == index) {
					continue;
				}
				// We can only upgrade a chunk if all its neighbours
				// within the radius are in at least the same state.
				if (chunkMap.getChunkState(neighIndex) < minNeighState) {
					upgradeSelf = false;
					queue.push(makePrioritized(prioIndex.priority, neighIndex));
				}
			}
		}
	}
	if (upgradeSelf) {
		enqueueUpgrade(chunkMap[index]);
	}
	return upgradeSelf;
}

void ChunkManager::enqueueUpgrade(ChunkPtr chunk) {
	switch (chunk->getState()) {
		case Chunk::NEW: enqueueGeneration(chunk); break;
		case Chunk::GENERATED: enqueueTesselation(chunk); break;
		default: BOOST_ASSERT(false);
	}
}

void ChunkManager::enqueueGeneration(ChunkPtr chunk) {
	BOOST_ASSERT(chunk->getState() == Chunk::NEW);
	BOOST_ASSERT(!chunk->isUpgrading());

	chunk->startUpgrade();
	threadPool.enqueue(
			boost::bind(
				&ChunkManager::generate, this,
				chunk->getIndex()));
}

void ChunkManager::enqueueTesselation(ChunkPtr chunk) {
	BOOST_ASSERT(chunk->getState() == Chunk::GENERATED);
	BOOST_ASSERT(!chunk->isUpgrading());

	int3 index = chunk->getIndex();
	chunk->startUpgrade();
	threadPool.enqueue(
			boost::bind(
				&ChunkManager::tesselate, this,
				index, boost::cref(chunkMap)));
}

void ChunkManager::generate(int3 index) {
	int3 position = chunkPositionFromIndex(index);

	RawChunkData rawChunkData;
	terrainGenerator->generateChunk(position, rawChunkData);

	OctreePtr octree(new Octree());
	buildOctree(rawChunkData, *octree);

	finalizerQueue.post(boost::bind(
				&ChunkManager::finalizeGeneration, this, index, octree));
}

void ChunkManager::finalizeGeneration(int3 index, OctreePtr octree) {
	ChunkPtr chunk = chunkMap[index];
	chunk->setOctree(octree);
	chunk->endUpgrade();
}

void ChunkManager::tesselate(int3 index, ChunkMap const &chunkMap) {
	ChunkGeometryPtr chunkGeometry(new ChunkGeometry());
	::tesselate(index, chunkMap, chunkGeometry);

	finalizerQueue.post(boost::bind(
				&ChunkManager::finalizeTesselation, this, index, chunkGeometry));
}

void ChunkManager::finalizeTesselation(int3 index, ChunkGeometryPtr chunkGeometry) {
	ChunkPtr chunk = chunkMap[index];
	chunk->setGeometry(chunkGeometry);
	chunk->endUpgrade();
}
