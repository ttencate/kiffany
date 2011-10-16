#include "terragen.h"

#include "chunk.h"
#include "chunkdata.h"
#include "stats.h"

#include <boost/bind.hpp>
#include <boost/random.hpp>

#include <cmath>

void TerrainGenerator::generateChunk(int3 const &position, ChunkDataPtr chunkData) const {
	SafeTimer::Timed t = stats.chunkGenerationTime.timed();
	doGenerateChunk(position, chunkData);
	stats.chunksGenerated.increment();
}

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

void PerlinTerrainGenerator::doGenerateChunk(int3 const &pos, ChunkDataPtr data) const {
	RleCompressor compressor(*data);
	for (unsigned z = 0; z < CHUNK_SIZE; ++z) {
		for (unsigned y = 0; y < CHUNK_SIZE; ++y) {
			for (unsigned x = 0; x < CHUNK_SIZE; ++x) {
				vec3 c = blockCenter(pos + int3(x, y, z));
				Block block = AIR_BLOCK;
				if (perlin(int3(c)) > 0.5f) {
					block = STONE_BLOCK;
				}
				compressor.put(block);
			}
		}
	}
}

float PerlinTerrainGenerator::lookup(int x, int y, int z) const {
	return noise[z * size * size + y * size + x];
}

float PerlinTerrainGenerator::perlin(int3 const &pos) const {
	float amplitude = 1.0f;
	float sum = 0.0f;
	float max = 0.0f;
	vec3 p(pos);
	for (unsigned i = 0; i < 7; ++i) {
		int3 index(mod((int)p.x, (int)size), mod((int)p.y, (int)size), mod((int)p.z, (int)size));
		int x = mod((int)floor(p.x), (int)size);
		int X = (x + 1) % size;
		int y = mod((int)floor(p.y), (int)size);
		int Y = (y + 1) % size;
		int z = mod((int)floor(p.z), (int)size);
		int Z = (z + 1) % size;
		float a = p.x - x;
		float b = p.y - y;
		float c = p.z - z;
		sum += amplitude * mix(
			mix(
				mix(lookup(x, y, z), lookup(X, y, z), a),
				mix(lookup(x, Y, z), lookup(X, Y, z), a),
				b),
			mix(
				mix(lookup(x, y, Z), lookup(X, y, Z), a),
				mix(lookup(x, Y, Z), lookup(X, Y, Z), a),
				b),
			c);
		max += amplitude;
		amplitude *= 2;
		p /= 2;
	}
	return sum / max;
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
