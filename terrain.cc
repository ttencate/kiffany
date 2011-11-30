#include "terrain.h"

#include "flags.h"
#include "occlusion.h"
#include "stats.h"
#include "terragen.h"

#include <queue>

ChunkManager::ChunkManager(ChunkMap &chunkMap, TerrainGenerator *terrainGenerator)
:
	chunkMap(chunkMap),
	terrainGenerator(terrainGenerator),
	finalizerQueue(ThreadPool::defaultNumThreads()),
	threadPool(ThreadPool::defaultNumThreads())
{
}

// TODO refactor these to be nonblocking
// TODO refactor Chunk::State to be an integer, combined with an "upgrading" boolean,
// and make a generic "upgrade" function
void ChunkManager::requestGeneration(ChunkPtr chunk) {
	if (chunk->getState() == Chunk::NEW) {
		chunk->setGenerating();
		threadPool.enqueue(
				boost::bind(
					&ChunkManager::generate, this,
					chunk->getIndex()));
	}
}

void ChunkManager::requestTesselation(ChunkPtr chunk) {
	if (chunk->getState() == Chunk::GENERATED) {
		int3 index = chunk->getIndex();
		NeighbourChunkData neighbourChunkData = getNeighbourChunkData(index);
		if (neighbourChunkData.isComplete()) {
			chunk->setTesselating();
			threadPool.enqueue(
					boost::bind(
						&ChunkManager::tesselate, this,
						index, chunk->getData(), neighbourChunkData));
		}
	}
}

void ChunkManager::requestLighting(ChunkPtr chunk) {
	if (chunk->getState() >= Chunk::TESSELATED && chunk->getState() != Chunk::LIGHTING) {
		int3 index = chunk->getIndex();
		chunk->setLighting();
		ChunkGeometryConstPtr chunkGeometry = chunk->getGeometry();
		threadPool.enqueue(
				boost::bind(
					&ChunkManager::computeLighting, this,
					index, boost::cref(chunkMap), chunkGeometry));
	}
}

void ChunkManager::addViewSphere(WeakConstViewSpherePtr viewSphere) {
	viewSpheres.push_back(viewSphere);
}

// TODO move elsewhere
template<typename T>
struct Prioritized {
	float priority; // lower number is higher priority
	T item;
	Prioritized(float priority, T item) : priority(priority), item(item) { }
	bool operator<(Prioritized<T> const &other) const {
		return priority > other.priority;
	}
};

template<typename T>
Prioritized<T> makePrioritized(float priority, T item) { return Prioritized<T>(priority, item); }

void ChunkManager::sow() {
	unsigned jobsToAdd = threadPool.getMaxQueueSize() - threadPool.getQueueSize();
	if (jobsToAdd == 0) {
		return;
	}

	std::priority_queue<Prioritized<int3> > pq;
	for (unsigned i = 0; i < viewSpheres.size(); ++i) {
		if (ConstViewSpherePtr sphere = viewSpheres[i].lock()) {
			// TODO write chunksInSphere iterator
			// TODO move some of this to ChunkMap to lock more coarsely
			int3 min = chunkIndexFromPoint(sphere->center - sphere->radius);
			int3 max = chunkIndexFromPoint(sphere->center + sphere->radius);
			for (int z = min.z; z <= max.z; ++z) {
				for (int y = min.y; y <= max.y; ++y) {
					for (int x = min.x; x <= max.x; ++x) {
						// TODO filter on the sphere
						int3 index(x, y, z);
						float priority = length(chunkCenter(index) - sphere->center); // lower is better
						if (chunkMap.getChunkState(index) < Chunk::LIGHTED) {
							pq.push(makePrioritized(priority, index));
						}
					}
				}
			}
		} else {
			viewSpheres.erase(viewSpheres.begin() + i);
		}
	}

	while (jobsToAdd && !pq.empty()) {
		int3 index = pq.top().item;
		pq.pop();
		Chunk::State state = chunkMap.getChunkState(index);
		if (state <= Chunk::NEW) {
			requestGeneration(chunkMap[index]);
			--jobsToAdd;
			continue;
		} else if (state == Chunk::GENERATED) {
			bool canTesselate = true;
			// TODO get from lighting radius
			int3 min = index - 1;
			int3 max = index + 1;
			// TODO write chunksInCube iterator
			for (int z = min.z; z <= max.z; ++z) {
				for (int y = min.y; y <= max.y; ++y) {
					for (int x = min.x; x <= max.x; ++x) {
						int3 neighIndex(x, y, z);
						if (neighIndex == index) {
							continue;
						}
						Chunk::State neighState = chunkMap.getChunkState(neighIndex);
						if (neighState < Chunk::GENERATING) {
							canTesselate = false;
							requestGeneration(chunkMap[neighIndex]);
							--jobsToAdd;
							if (!jobsToAdd) {
								return;
							}
						}
					}
				}
			}
			if (canTesselate) {
				requestTesselation(chunkMap[index]);
				--jobsToAdd;
			}
		}
	}
}

void ChunkManager::reap() {
	finalizerQueue.runAll();
}

ChunkDataPtr ChunkManager::chunkDataOrNull(int3 index) {
	ChunkPtr chunk = chunkMap[index];
	if (!chunk) {
		return ChunkDataPtr();
	}
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
	ChunkPtr chunk = chunkMap[index];
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

void Terrain::addViewSphere(WeakConstViewSpherePtr viewSphere) {
	chunkManager.addViewSphere(viewSphere);
}

void Terrain::update(float dt) {
	chunkManager.sow();
	chunkManager.reap();
}

void Terrain::render(Camera const &camera) {
	float ambientColor[] = { 0.5f, 0.5f, 0.5f, 1.0f };
	float diffuseColor[] = { 0.5f, 0.5f, 0.5f, 1.0f };
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, ambientColor);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, diffuseColor);

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);

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
