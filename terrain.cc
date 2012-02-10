#include "terrain.h"

#include "flags.h"
#include "lighting.h"
#include "occlusion.h"
#include "stats.h"
#include "terragen.h"

#include <boost/assert.hpp>

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
	if (state >= Chunk::LIGHTED) {
		return 0;
	}

	Chunk::State const nextState = (Chunk::State)(state + 1);
	int radius;
	// TODO replace by helper function
	switch (nextState) {
		case Chunk::GENERATED: radius = 0; break;
		case Chunk::TESSELATED: radius = 1; break;
		case Chunk::LIGHTED: radius = 0; break; // TODO get from lighting radius
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

OctreePtr ChunkManager::octreeOrNull(int3 index) {
	ChunkPtr chunk = chunkMap[index];
	if (!chunk) {
		return OctreePtr();
	}
	return chunk->getOctree();
}

// TODO get rid of NeighbourOctrees, read chunk map directly
NeighbourOctrees ChunkManager::getNeighbourOctrees(int3 index) {
	NeighbourOctrees octrees;
	octrees.xn = octreeOrNull(index + int3(-1,  0,  0));
	octrees.xp = octreeOrNull(index + int3( 1,  0,  0));
	octrees.yn = octreeOrNull(index + int3( 0, -1,  0));
	octrees.yp = octreeOrNull(index + int3( 0,  1,  0));
	octrees.zn = octreeOrNull(index + int3( 0,  0, -1));
	octrees.zp = octreeOrNull(index + int3( 0,  0,  1));
	return octrees;
}

void ChunkManager::enqueueUpgrade(ChunkPtr chunk) {
	switch (chunk->getState()) {
		case Chunk::NEW: enqueueGeneration(chunk); break;
		case Chunk::GENERATED: enqueueTesselation(chunk); break;
		case Chunk::TESSELATED: enqueueLighting(chunk); break;
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
	NeighbourOctrees neighbourOctrees = getNeighbourOctrees(index);
	BOOST_ASSERT(neighbourOctrees.isComplete());

	chunk->startUpgrade();
	threadPool.enqueue(
			boost::bind(
				&ChunkManager::tesselate, this,
				index, chunk->getOctree(), neighbourOctrees));
}

void ChunkManager::enqueueLighting(ChunkPtr chunk) {
	BOOST_ASSERT(chunk->getState() == Chunk::TESSELATED);
	BOOST_ASSERT(!chunk->isUpgrading());

	int3 index = chunk->getIndex();
	chunk->startUpgrade();
	ChunkGeometryConstPtr chunkGeometry = chunk->getGeometry();
	threadPool.enqueue(
			boost::bind(
				&ChunkManager::computeLighting, this,
				index, boost::cref(chunkMap), chunkGeometry));
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

void ChunkManager::tesselate(int3 index, OctreePtr octree, NeighbourOctrees neighbourOctrees) {
	ChunkGeometryPtr chunkGeometry(new ChunkGeometry());
	::tesselate(octree, neighbourOctrees, chunkGeometry);

	finalizerQueue.post(boost::bind(
				&ChunkManager::finalizeTesselation, this, index, chunkGeometry));
}

void ChunkManager::finalizeTesselation(int3 index, ChunkGeometryPtr chunkGeometry) {
	ChunkPtr chunk = chunkMap[index];
	chunk->setGeometry(chunkGeometry);
	chunk->endUpgrade();
}

void ChunkManager::computeLighting(int3 index, ChunkMap const &chunkMap, ChunkGeometryConstPtr chunkGeometry) {
	// TODO Split out vertex and normal data
	ChunkGeometryPtr newGeometry(new ChunkGeometry(*chunkGeometry));
	::computeLighting(index, chunkMap, newGeometry);

	finalizerQueue.post(boost::bind(
				&ChunkManager::finalizeLighting, this, index, newGeometry));
}

void ChunkManager::finalizeLighting(int3 index, ChunkGeometryPtr chunkGeometry) {
	ChunkPtr chunk = chunkMap[index];
	if (!chunk) {
		stats.irrelevantJobsRun.increment();
		return;
	}
	chunk->setGeometry(chunkGeometry);
	chunk->endUpgrade();
}

Terrain::Terrain(TerrainGenerator *terrainGenerator)
:
	chunkMap(computeMaxNumChunks()),
	chunkManager(chunkMap, terrainGenerator)
{
	shaderProgram.loadAndLink("shaders/terrain.vert", "shaders/terrain.frag");
}

Terrain::~Terrain() {
}

void Terrain::addViewSphere(WeakConstViewSpherePtr viewSphere) {
	chunkManager.addViewSphere(viewSphere);
}

void Terrain::update(float dt) {
	chunkManager.sow();
	chunkManager.reap();
}

void Terrain::render(Camera const &camera, Lighting const &lighting) {
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);

	useProgram(shaderProgram.getProgram());
	shaderProgram.setUniform("material.ambient", vec4(0.5f, 0.5f, 0.5f, 1.0f));
	shaderProgram.setUniform("material.diffuse", vec4(0.5f, 0.5f, 0.5f, 1.0f));
	shaderProgram.setUniform("lighting.ambientColor", lighting.ambientColor());
	shaderProgram.setUniform("lighting.sunColor", lighting.sunColor());
	shaderProgram.setUniform("lighting.sunDirection", lighting.sunDirection());
	bindFragDataLocation(shaderProgram.getProgram(), 0, "color");

	int3 center = chunkIndexFromPoint(camera.getPosition());
	int radius = flags.viewDistance / CHUNK_SIZE;
	for (int z = -radius; z <= radius; ++z) {
		for (int y = -radius; y <= radius; ++y) {
			for (int x = -radius; x <= radius; ++x) {
				// TODO sphere check
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
	// TODO avoid creating 'em (change [] semantics?)
	ChunkPtr chunk = chunkMap[index];
	if (!chunk) {
		return;
	}
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
