#include "raycaster.h"

#include "block.h"
#include "chunkmap.h"

#include <boost/test/unit_test.hpp>

namespace {
	struct Fixture {
		ChunkMap chunkMap;
		Raycaster raycaster;

		Fixture()
		:
			chunkMap(),
			raycaster(chunkMap, CHUNK_SIZE, STONE_BLOCK, BLOCK_MASK)
		{
		}

		RaycastResult cast(vec3 positionInStartChunk, vec3 direction) {
			return raycaster(int3(0, 0, 0), positionInStartChunk, normalize(direction));
		}

		void testIndeterminate(vec3 positionInStartChunk, vec3 direction) {
			RaycastResult result = cast(positionInStartChunk, direction);
			BOOST_TEST_CHECKPOINT("Checking indeterminate: "
					<< glm::to_string(positionInStartChunk) << " "
					<< glm::to_string(direction));
			BOOST_CHECK_EQUAL(RaycastResult::INDETERMINATE, result.status);
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

BOOST_AUTO_TEST_SUITE_END()
