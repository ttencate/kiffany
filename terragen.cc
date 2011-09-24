#include "chunkdata.h"
#include "terragen.h"

#include <boost/random.hpp>

#include <cmath>

PerlinTerrainGenerator::PerlinTerrainGenerator(unsigned size, unsigned seed)
:
	size(size),
	noise(new float[size * size * size])
{
	boost::mt11213b engine(seed);
	boost::uniform_01<float> uniform;
	for (unsigned i = 0; i < size * size * size; ++i) {
		noise[i] = uniform(engine);
	}
}

void PerlinTerrainGenerator::generateChunk(ChunkData &data, int3 const &pos) const {
	CoordsBlock coordsBlock = data.getCoordsBlock();
	for (CoordsBlock::const_iterator i = coordsBlock.begin(); i != coordsBlock.end(); ++i) {
		if (perlin(*i) > 0.5f) {
			data[*i] = STONE_BLOCK;
		} else {
			data[*i] = AIR_BLOCK;
		}
	}
}

float PerlinTerrainGenerator::perlin(int3 const &pos) const {
	float amplitude = 1.0f;
	float period = 1.0f;
	float sum = 0.0f;
	float max = 0.0f;
	vec3 p(pos);
	for (unsigned i = 0; i < 4; ++i) {
		int3 index(mod((int)p.x, (int)size), mod((int)p.y, (int)size), mod((int)p.z, (int)size));
		sum += amplitude * noise[index.z * size * size + index.y * size + index.x];
		max += amplitude;
		amplitude *= 2;
		p /= 2;
	}
	return sum / max;
}

void SineTerrainGenerator::generateChunk(ChunkData &data, int3 const &pos) const {
	float const amplitude = 32.0f;
	float const period = 256.0f;
	float const omega = 2 * M_PI / period;
	CoordsBlock coordsBlock = data.getCoordsBlock();
	for (CoordsBlock::const_iterator i = coordsBlock.begin(); i != coordsBlock.end(); ++i) {
		vec3 c = blockCenter(pos + *i);
		if (c.z > amplitude * (sinf(omega * c.x) + sinf(omega * c.y))) {
			data[*i] = AIR_BLOCK;
		} else {
			data[*i] = STONE_BLOCK;
		}
	}
}
