#include "terragen.h"

#include "chunk.h"
#include "chunkdata.h"
#include "stats.h"

#include <boost/bind.hpp>
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

void PerlinTerrainGenerator::generateChunk(int3 const &pos, ChunkData *data) const {
	CoordsBlock coordsBlock = data->getCoordsBlock();
	for (CoordsBlock::const_iterator i = coordsBlock.begin(); i != coordsBlock.end(); ++i) {
		if (perlin(*i) > 0.5f) {
			(*data)[*i] = STONE_BLOCK;
		} else {
			(*data)[*i] = AIR_BLOCK;
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

void SineTerrainGenerator::generateChunk(int3 const &pos, ChunkData *data) const {
	float const amplitude = 32.0f;
	float const period = 256.0f;
	float const omega = 2 * M_PI / period;
	Block *p = data->raw();
	for (unsigned z = 0; z < CHUNK_SIZE; ++z) {
		for (unsigned y = 0; y < CHUNK_SIZE; ++y) {
			for (unsigned x = 0; x < CHUNK_SIZE; ++x) {
				vec3 c = blockCenter(pos + int3(x, y, z));
				if (c.z > amplitude * (sinf(omega * c.x) + sinf(omega * c.y))) {
					*p = AIR_BLOCK;
				} else {
					*p = STONE_BLOCK;
				}
				++p;
			}
		}
	}
}

AsyncTerrainGenerator::AsyncTerrainGenerator(TerrainGenerator &terrainGenerator)
:
	terrainGenerator(terrainGenerator),
	threadPool(32, 32)
{
}

bool AsyncTerrainGenerator::tryGenerate(ChunkPtr chunk) {
	if (inProgress.find(chunk) != inProgress.end()) {
		return true;
	}
	ChunkGeometry* chunkGeometry = new ChunkGeometry(); // Caution!
	if (!threadPool.tryEnqueue(
		boost::bind(&AsyncTerrainGenerator::work, this, chunk->getPosition(), chunkGeometry),
		boost::bind(&AsyncTerrainGenerator::finalize, this, chunk, chunkGeometry))) {
		delete chunkGeometry;
		return false;
	} else {
		chunk->generating();
		inProgress.insert(chunk);
		return true;
	}
}

void AsyncTerrainGenerator::gather() {
	threadPool.runFinalizers();
}

void AsyncTerrainGenerator::work(int3 position, ChunkGeometry* chunkGeometry) {
	boost::scoped_ptr<ChunkData> chunkData(new ChunkData());
	// TODO move timing and counting into public nonvirtual method on TerrainGenerator
	{
		Timed t = stats.chunkGenerationTime.timed();
		terrainGenerator.generateChunk(position, chunkData.get());
	}
	stats.chunksGenerated.increment();

	tesselate(*chunkData, position, chunkGeometry);
}

void AsyncTerrainGenerator::finalize(ChunkPtr chunk, ChunkGeometry *chunkGeometry) {
	chunk->setGeometry(chunkGeometry);
	inProgress.erase(chunk);
}
