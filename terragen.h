#ifndef TERRAGEN_H
#define TERRAGEN_H

#include "chunkdata.h"
#include "maths.h"

#include <boost/noncopyable.hpp>
#include <boost/scoped_array.hpp>

// Must be thread-safe.
class TerrainGenerator {

	public:

		void generateChunk(int3 const &position, ChunkDataPtr chunkData) const;

	private:

		virtual void doGenerateChunk(int3 const &pos, ChunkDataPtr data) const = 0;

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

		virtual void doGenerateChunk(int3 const &pos, ChunkDataPtr data) const;

		float lookup(int x, int y, int z) const;
		float perlin(int3 const &pos) const;

};

class SineTerrainGenerator
:
	public TerrainGenerator
{

	private:

		virtual void doGenerateChunk(int3 const &pos, ChunkDataPtr data) const;

};

#endif
