#include "chunkdata.h"
#include "chunkmap.h"
#include "occlusion.h"
#include "octree.h"
#include "stats.h"
#include "terragen.h"

#include <sstream>

int main(int argc, char **argv) {
	int size = 5;
	if (argc >= 2) {
		std::istringstream ss(argv[1]);
		ss >> size;
	}
	int3 min = int3(-size / 2);
	int3 max = min + int3(size);
	std::cout << "Chunks from " << glm::to_string(min) << " to " << glm::to_string(max) << std::endl;

	ChunkMap chunkMap;
	PerlinTerrainGenerator generator(32 ,0);

	unsigned generated = 0;
	unsigned empty = 0;
	unsigned full = 0;
	for (int z = min.z; z < max.z; ++z) {
		for (int y = min.y; y < max.y; ++y) {
			for (int x = min.x; x < max.x; ++x) {
				int3 const index = int3(x, y, z);

				RawChunkData chunkData;
				generator.generateChunk(chunkPositionFromIndex(index), chunkData);

				OctreePtr octree(new Octree());
				buildOctree(chunkData, *octree);

				ChunkPtr chunk = chunkMap[index];
				chunk->setOctree(octree);

				++generated;
				if (octree->isEmpty()) {
					++empty;
					std::cout << "E" << std::flush;
				} else if (octree->getNodes().size() == 1) {
					++full;
					std::cout << "F" << std::flush;
				} else {
					std::cout << "G" << std::flush;
				}
			}
		}
	}
	std::cout
		<< '\n'
		<< "Generated " << generated << " total, " << empty << " empty" << ", " << full << " full" << std::endl;

	unsigned tesselated = 0;
	for (int z = min.z; z < max.z; ++z) {
		for (int y = min.y; y < max.y; ++y) {
			for (int x = min.x; x < max.x; ++x) {
				int3 const index = int3(x, y, z);
				ChunkPtr chunk = chunkMap[index];
				OctreePtr octree = chunk->getOctree();

				NeighbourOctrees neighbourOctrees;
				neighbourOctrees.xn = chunkMap.getOctreeOrNull(index + int3(-1,  0,  0));
				neighbourOctrees.xp = chunkMap.getOctreeOrNull(index + int3( 1,  0,  0));
				neighbourOctrees.yn = chunkMap.getOctreeOrNull(index + int3( 0, -1,  0));
				neighbourOctrees.yp = chunkMap.getOctreeOrNull(index + int3( 0,  1,  0));
				neighbourOctrees.zn = chunkMap.getOctreeOrNull(index + int3( 0,  0, -1));
				neighbourOctrees.zp = chunkMap.getOctreeOrNull(index + int3( 0,  0,  1));
				if (neighbourOctrees.isComplete()) {
					ChunkGeometryPtr chunkGeometry(new ChunkGeometry());
					::tesselate(octree, neighbourOctrees, chunkGeometry);
					chunk->setGeometry(chunkGeometry);
					++tesselated;
					std::cout << "T" << std::flush;
				} else {
					std::cout << "." << std::flush;
				}
			}
		}
	}
	std::cout
		<< '\n'
		<< "Tesselated " << tesselated << std::endl;

	unsigned lit = 0;
	for (int z = min.z; z < max.z; ++z) {
		for (int y = min.y; y < max.y; ++y) {
			for (int x = min.x; x < max.x; ++x) {
				int3 const index = int3(x, y, z);
				ChunkPtr chunk = chunkMap[index];
				ChunkGeometryConstPtr chunkGeometry = chunk->getGeometry();
				if (chunkGeometry) {
					ChunkGeometryPtr newChunkGeometry(new ChunkGeometry(*chunkGeometry));
					computeLighting(index, chunkMap, newChunkGeometry);
					chunk->setGeometry(newChunkGeometry);
					++lit;
					std::cout << "L" << std::flush;
				} else {
					std::cout << "." << std::flush;
				}
			}
		}
	}
	std::cout
		<< '\n'
		<< "Lit " << lit << std::endl;

	stats.print();
}
