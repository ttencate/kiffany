#include "terragen.h"

#include "chunk.h"
#include "chunkdata.h"
#include "stats.h"

#include <boost/bind.hpp>
#include <boost/random.hpp>
#include <boost/random/normal_distribution.hpp>

#include <cmath>

void TerrainGenerator::generateChunk(int3 const &position, RawChunkData &rawChunkData) const {
	SafeTimer::Timed t = stats.chunkGenerationTime.timed();
	doGenerateChunk(position, rawChunkData);
	stats.chunksGenerated.increment();
}

// TODO don't reuse seed
PerlinTerrainGenerator::PerlinTerrainGenerator(unsigned size, unsigned seed)
:
	perlin2D(Noise2D(size, seed), buildOctaves2D(seed)),
	perlin3D(Noise3D(size, seed), buildOctaves3D(seed))
{
}

Octaves PerlinTerrainGenerator::buildOctaves2D(unsigned seed) const {
	boost::mt11213b engine(seed);
	boost::normal_distribution<float> normal(1.0f, 0.1f);
	boost::variate_generator<boost::mt11213b, boost::normal_distribution<float> > gen(engine, normal);
	Octaves octaves;
	for (int i = 512; i >= 8; i /= 2) {
		float period = i * gen();
		float amplitude = 0.5f * i;
		octaves.push_back(Octave(period, amplitude));
	}
	return octaves;
}

Octaves PerlinTerrainGenerator::buildOctaves3D(unsigned seed) const {
	boost::mt11213b engine(seed);
	boost::normal_distribution<float> normal(1.0f, 0.1f);
	boost::variate_generator<boost::mt11213b, boost::normal_distribution<float> > gen(engine, normal);
	Octaves octaves;
	for (int i = 64; i >= 8; i /= 2) {
		float period = i * gen();
		float amplitude = i;
		octaves.push_back(Octave(period, amplitude));
	}
	return octaves;
}

void PerlinTerrainGenerator::doGenerateChunk(int3 const &pos, RawChunkData &rawChunkData) const {
	std::vector<float> heights(CHUNK_SIZE * CHUNK_SIZE);
	for (unsigned y = 0; y < CHUNK_SIZE; ++y) {
		for (unsigned x = 0; x < CHUNK_SIZE; ++x) {
			vec2 p(blockCenter(pos + int3(x, y, 0)));
			heights[x + CHUNK_SIZE * y] = perlin2D(p);
		}
	}
	float const amplitude3D = perlin3D.getAmplitude();
	Block *p = rawChunkData.raw();
	for (unsigned z = 0; z < CHUNK_SIZE; ++z) {
		for (unsigned y = 0; y < CHUNK_SIZE; ++y) {
			for (unsigned x = 0; x < CHUNK_SIZE; ++x) {
				vec3 center = blockCenter(pos + int3(x, y, z));
				Block block = AIR_BLOCK;
				float h = center.z - heights[x + CHUNK_SIZE * y];
				if (h >= -amplitude3D && h <= 0) {
					h += perlin3D(center);
				}
				if (h < 0) {
					block = STONE_BLOCK;
				}
				*p = block;
				++p;
			}
		}
	}
}

void SineTerrainGenerator::doGenerateChunk(int3 const &pos, RawChunkData &rawChunkData) const {
	float const amplitude = 32.0f;
	float const period = 256.0f;
	float const omega = 2 * M_PI / period;
	Block *p = rawChunkData.raw();
	for (unsigned z = 0; z < CHUNK_SIZE; ++z) {
		for (unsigned y = 0; y < CHUNK_SIZE; ++y) {
			for (unsigned x = 0; x < CHUNK_SIZE; ++x) {
				vec3 c = blockCenter(pos + int3(x, y, z));
				Block block = AIR_BLOCK;
				if (c.z < amplitude * (sinf(omega * c.x) + sinf(omega * c.y))) {
					block = STONE_BLOCK;
				}
				*p = block;
				++p;
			}
		}
	}
}
