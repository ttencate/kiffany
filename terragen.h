#ifndef TERRAGEN_H
#define TERRAGEN_H

#include "chunkdata.h"
#include "maths.h"
#include "perlin.h"

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

	Perlin<Noise2D> perlin2D;
	Perlin<Noise3D> perlin3D;

	public:

		PerlinTerrainGenerator(unsigned size, unsigned seed);

	private:

		Octaves buildOctaves2D(unsigned seed) const;
		Octaves buildOctaves3D(unsigned seed) const;

		virtual void doGenerateChunk(int3 const &pos, ChunkDataPtr data) const;

};

class SineTerrainGenerator
:
	public TerrainGenerator
{

	private:

		virtual void doGenerateChunk(int3 const &pos, ChunkDataPtr data) const;

};

#endif
