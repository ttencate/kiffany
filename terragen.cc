#include "chunkdata.h"
#include "terragen.h"

#include <cmath>

void SineTerrainGenerator::generateChunk(ChunkData &data, int3 const &pos) const {
	float const amplitude = 1.0f * CHUNK_SIZE;
	float const period = 4.0f * CHUNK_SIZE;
	float const omega = 2 * M_PI / period;
	for (ChunkData::coords_iterator i = data.beginCoords(); i != data.endCoords(); ++i) {
		vec3 c = blockCenter(pos + *i);
		if (c.z > amplitude * (sinf(omega * c.x) + sinf(omega * c.y))) {
			data[*i] = AIR_BLOCK;
		} else {
			data[*i] = STONE_BLOCK;
		}
	}
}
