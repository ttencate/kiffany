#include "terragen.h"

#include "chunk.h"
#include "chunkdata.h"
#include "stats.h"

#include <boost/bind.hpp>
#include <boost/random.hpp>
#include <boost/random/normal_distribution.hpp>

#include <cmath>
#include <iostream> // TODO remove

void TerrainGenerator::generateChunk(int3 const &position, ChunkDataPtr chunkData) const {
	SafeTimer::Timed t = stats.chunkGenerationTime.timed();
	doGenerateChunk(position, chunkData);
	stats.chunksGenerated.increment();
}

PerlinTerrainGenerator::PerlinTerrainGenerator(unsigned size, unsigned seed)
:
	perlin(Noise2D(size, seed), buildOctaves(seed))
{
}

Octaves PerlinTerrainGenerator::buildOctaves(unsigned seed) const {
	boost::mt11213b engine(seed);
	boost::normal_distribution<float> normal(1.0f, 0.1f);
	boost::variate_generator<boost::mt11213b, boost::normal_distribution<float> > gen(engine, normal);
	Octaves octaves;
	for (int i = 1024; i >= 8; i /= 2) {
		float period = i * gen();
		std::cerr << period << '\n';
		float amplitude = 0.5f * i;
		octaves.push_back(Octave(period, amplitude));
	}
	return octaves;
}

void PerlinTerrainGenerator::doGenerateChunk(int3 const &pos, ChunkDataPtr data) const {
	RleCompressor compressor(*data);
	for (unsigned z = 0; z < CHUNK_SIZE; ++z) {
		for (unsigned y = 0; y < CHUNK_SIZE; ++y) {
			for (unsigned x = 0; x < CHUNK_SIZE; ++x) {
				vec3 center = blockCenter(pos + int3(x, y, z));
				vec2 pos(center.x, center.y);
				Block block = AIR_BLOCK;
				if (center.z < perlin(pos)) {
					block = STONE_BLOCK;
				}
				compressor.put(block);
			}
		}
	}
}

void SineTerrainGenerator::doGenerateChunk(int3 const &pos, ChunkDataPtr data) const {
	float const amplitude = 32.0f;
	float const period = 256.0f;
	float const omega = 2 * M_PI / period;
	RleCompressor compressor(*data);
	for (unsigned z = 0; z < CHUNK_SIZE; ++z) {
		for (unsigned y = 0; y < CHUNK_SIZE; ++y) {
			for (unsigned x = 0; x < CHUNK_SIZE; ++x) {
				vec3 c = blockCenter(pos + int3(x, y, z));
				Block block = AIR_BLOCK;
				if (c.z < amplitude * (sinf(omega * c.x) + sinf(omega * c.y))) {
					block = STONE_BLOCK;
				}
				compressor.put(block);
			}
		}
	}
}
