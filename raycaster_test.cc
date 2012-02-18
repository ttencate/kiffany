#include "raycaster.h"

#include "block.h"
#include "chunkdata.h"
#include "chunkmap.h"
#include "octree.h"

#include <boost/test/unit_test.hpp>

namespace {
	struct Fixture {
		ChunkMap chunkMap;
		Raycaster raycaster;

		Fixture()
		:
			chunkMap(),
			raycaster(chunkMap, CHUNK_SIZE - 1, STONE_BLOCK, BLOCK_MASK)
		{
		}

		RaycastResult cast(vec3 positionInStartChunk, vec3 direction) {
			return raycaster(int3(0, 0, 0), positionInStartChunk, normalize(direction));
		}

		void testCutoff(vec3 positionInStartChunk, vec3 direction) {
			BOOST_TEST_MESSAGE("Checking cutoff: "
					<< glm::to_string(positionInStartChunk) << " "
					<< glm::to_string(direction));
			RaycastResult result = cast(positionInStartChunk, direction);
			BOOST_CHECK_EQUAL(RaycastResult::CUTOFF, result.status);
			BOOST_CHECK_LE(raycaster.getCutoff(), result.length);
		}

		void testHit(vec3 positionInStartChunk, vec3 direction, float expectedLength) {
			BOOST_TEST_MESSAGE("Checking hit: "
					<< glm::to_string(positionInStartChunk) << " "
					<< glm::to_string(direction) << " "
					<< expectedLength);
			RaycastResult result = cast(positionInStartChunk, direction);
			BOOST_CHECK_EQUAL(RaycastResult::HIT, result.status);
			BOOST_CHECK_CLOSE(expectedLength, result.length, 1e-2f);
		}

		void testIndeterminate(vec3 positionInStartChunk, vec3 direction) {
			BOOST_TEST_MESSAGE("Checking indeterminate: "
					<< glm::to_string(positionInStartChunk) << " "
					<< glm::to_string(direction));
			RaycastResult result = cast(positionInStartChunk, direction);
			BOOST_CHECK_EQUAL(RaycastResult::INDETERMINATE, result.status);
		}

		void fillBlock(ChunkPtr chunk, int3 min, int3 max, Block block = STONE_BLOCK) {
			RawChunkData chunkData;
			for (int z = min.z; z < max.z; ++z) {
				for (int y = min.y; y < max.y; ++y) {
					for (int x = min.x; x < max.x; ++x) {
						chunkData[int3(x, y, z)] = block;
					}
				}
			}
			OctreePtr octree(new Octree());
			buildOctree(chunkData, *octree);
			chunk->setOctree(octree);
		}
	};
}

BOOST_FIXTURE_TEST_SUITE(RaycasterTest, Fixture)

BOOST_AUTO_TEST_CASE(TestEmpty) {
	for (unsigned hz = 0; hz < 6; ++hz) {
		for (unsigned hy = 0; hy < 6; ++hy) {
			for (unsigned hx = 0; hx < 6; ++hx) {
				vec3 pos(0.2f * CHUNK_SIZE * hx, 0.2f * CHUNK_SIZE * hy, 0.2f * CHUNK_SIZE * hz);
				for (int dz = -1; dz <= 1; ++dz) {
					for (int dy = -1; dy <= 1; ++dy) {
						for (int dx = -1; dx <= 1; ++dx) {
							if (!dx && !dy && !dz)
								continue;
							vec3 direction(dx, dy, dz);
							testIndeterminate(pos + 0.1f * direction, direction);
						}
					}
				}
			}
		}
	}
}

BOOST_AUTO_TEST_CASE(TestSurroundedByAir) {
	for (int z = -1; z <= 1; ++z) {
		for (int y = -1; y <= 1; ++y) {
			for (int x = -1; x <= 1; ++x) {
				fillBlock(chunkMap[int3(x, y, z)], int3(0, 0, 0), int3(CHUNK_SIZE, CHUNK_SIZE, CHUNK_SIZE), AIR_BLOCK);
			}
		}
	}
	for (unsigned hz = 0; hz < 6; ++hz) {
		for (unsigned hy = 0; hy < 6; ++hy) {
			for (unsigned hx = 0; hx < 6; ++hx) {
				vec3 pos(0.2f * CHUNK_SIZE * hx, 0.2f * CHUNK_SIZE * hy, 0.2f * CHUNK_SIZE * hz);
				for (int dz = -1; dz <= 1; ++dz) {
					for (int dy = -1; dy <= 1; ++dy) {
						for (int dx = -1; dx <= 1; ++dx) {
							if (!dx && !dy && !dz)
								continue;
							vec3 direction(dx, dy, dz);
							testCutoff(pos + 0.1f * direction, direction);
						}
					}
				}
			}
		}
	}
}

BOOST_AUTO_TEST_CASE(TestSurroundedByStone) {
	for (int z = -1; z <= 1; ++z) {
		for (int y = -1; y <= 1; ++y) {
			for (int x = -1; x <= 1; ++x) {
				Block block = x || y || z ? STONE_BLOCK : AIR_BLOCK;
				fillBlock(chunkMap[int3(x, y, z)], int3(0, 0, 0), int3(CHUNK_SIZE, CHUNK_SIZE, CHUNK_SIZE), block);
			}
		}
	}
	vec3 pos(0.5f * CHUNK_SIZE, 0.5f * CHUNK_SIZE, 0.5f * CHUNK_SIZE);
	for (int dz = -1; dz <= 1; ++dz) {
		for (int dy = -1; dy <= 1; ++dy) {
			for (int dx = -1; dx <= 1; ++dx) {
				if (!dx && !dy && !dz)
					continue;
				vec3 direction(dx, dy, dz);
				testHit(pos, direction, length(0.5f * CHUNK_SIZE * direction));
			}
		}
	}
}

BOOST_AUTO_TEST_SUITE_END()
