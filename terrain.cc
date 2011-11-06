#include "terrain.h"

#include "flags.h"
#include "occlusion.h"
#include "stats.h"
#include "terragen.h"

ChunkManager::ChunkManager(ChunkMap &chunkMap, TerrainGenerator *terrainGenerator)
:
	chunkMap(chunkMap),
	terrainGenerator(terrainGenerator),
	finalizerQueue(JobThreadPool::defaultNumThreads()),
	threadPool(0)
{
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

void ChunkManager::requestLighting(ChunkPtr chunk) {
	if (chunk->getState() >= Chunk::TESSELATED && chunk->getState() != Chunk::LIGHTING) {
		int3 index = chunk->getIndex();
		chunk->setLighting();
		ChunkGeometryConstPtr chunkGeometry = chunk->getGeometry();
		threadPool.enqueue(Job(
					index,
					1.0e-6f,
					boost::bind(
						&ChunkManager::computeLighting, this,
						index, boost::cref(chunkMap), chunkGeometry)));
	}
}

void ChunkManager::setPriorityFunction(ChunkPriorityFunction const &priorityFunction) {
	threadPool.setPriorityFunction(
			JobThreadPool::PriorityFunction(JobPriorityFunction(priorityFunction)));
}

void ChunkManager::gather() {
	finalizerQueue.runAll();
	stats.irrelevantJobsSkipped.increment(
			threadPool.cleanUp(boost::bind(&ChunkManager::isJobIrrelevant, this, _1)));
}

ChunkDataPtr ChunkManager::chunkDataOrNull(int3 index) {
	ChunkPtr chunk = chunkMap[index];
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
	ChunkPtr chunk = chunkMap[index];
	if (!chunk) {
		stats.irrelevantJobsRun.increment();
		return;
	}
	chunk->setData(chunkData);
	chunk->setOctree(octree);

	int const R = 1;
	for (int z = -R; z <= R; ++z) {
		for (int y = -R; y <= R; ++y) {
			for (int x = -R; x <= R; ++x) {
				ChunkPtr chunk = chunkMap[index + int3(x, y, z)];
				if (chunk) {
					requestLighting(chunk);
				}
			}
		}
	}
}

void ChunkManager::tesselate(int3 index, ChunkDataPtr chunkData, NeighbourChunkData neighbourChunkData) {
	ChunkGeometryPtr chunkGeometry(new ChunkGeometry());
	::tesselate(chunkData, neighbourChunkData, chunkGeometry);

	finalizerQueue.post(boost::bind(
				&ChunkManager::finalizeTesselation, this, index, chunkGeometry));
}

void ChunkManager::finalizeTesselation(int3 index, ChunkGeometryPtr chunkGeometry) {
	ChunkPtr chunk = chunkMap[index];
	if (!chunk) {
		stats.irrelevantJobsRun.increment();
		return;
	}
	chunk->setGeometry(chunkGeometry);
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
}

Terrain::Terrain(TerrainGenerator *terrainGenerator)
:
	chunkMap(computeMaxNumChunks()),
	chunkManager(chunkMap, terrainGenerator)
{
}

Terrain::~Terrain() {
}

void Terrain::update(float dt) {
	chunkManager.gather();
}

void Terrain::render(Camera const &camera) {
	float ambientColor[] = { 0.5f, 0.5f, 0.5f, 1.0f };
	float diffuseColor[] = { 0.5f, 0.5f, 0.5f, 1.0f };
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, ambientColor);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, diffuseColor);

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);

	ChunkPriorityFunction priorityFunction(camera);
	chunkMap.setPriorityFunction(priorityFunction);
	chunkManager.setPriorityFunction(priorityFunction);

	int3 center = chunkIndexFromPoint(camera.getPosition());
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
	ChunkPtr chunk = chunkMap[index];
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
