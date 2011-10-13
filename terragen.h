#ifndef TERRAGEN_H
#define TERRAGEN_H

#include "maths.h"

#include <boost/noncopyable.hpp>
#include <boost/scoped_array.hpp>

class ChunkData;
class ChunkGeometry;

// Must be thread-safe.
class TerrainGenerator {

	public:

		void generateChunk(int3 const &position, ChunkData *chunkData) const;

	private:

		virtual void doGenerateChunk(int3 const &pos, ChunkData *data) const = 0;

};

class PerlinTerrainGenerator
:
	boost::noncopyable,
	public TerrainGenerator
{

	unsigned const size;
	boost::scoped_array<float> noise;

	public:

		PerlinTerrainGenerator(unsigned size, unsigned seed);

	private:

		virtual void doGenerateChunk(int3 const &pos, ChunkData *data) const;

		float lookup(int x, int y, int z) const;
		float perlin(int3 const &pos) const;

};

class SineTerrainGenerator
:
	public TerrainGenerator
{

	private:

		virtual void doGenerateChunk(int3 const &pos, ChunkData *data) const;

};

#endif
