#include "chunkdata.h"
#include "chunkmap.h"
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
	for (int z = min.z + 1; z < max.z - 1; ++z) {
		for (int y = min.y + 1; y < max.y - 1; ++y) {
			for (int x = min.x + 1; x < max.x - 1; ++x) {
				int3 const index = int3(x, y, z);
				ChunkPtr chunk = chunkMap[index];
				ChunkGeometryPtr chunkGeometry(new ChunkGeometry());
				::tesselate(index, chunkMap, chunkGeometry);
				chunk->setGeometry(chunkGeometry);

				++tesselated;
				std::cout << "T" << std::flush;
			}
		}
	}
	std::cout
		<< '\n'
		<< "Tesselated " << tesselated << std::endl;

	stats.print();
}
