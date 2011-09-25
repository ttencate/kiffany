#ifndef TERRAGEN_H
#define TERRAGEN_H

#include "maths.h"
#include "threadpool.h"

#include <boost/noncopyable.hpp>
#include <boost/scoped_array.hpp>

class Chunk;
class ChunkData;

class TerrainGenerator {

	public:

		virtual void generateChunk(ChunkData &data, int3 const &pos) const = 0;

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

		virtual void generateChunk(ChunkData &data, int3 const &pos) const;

	private:

		float lookup(int x, int y, int z) const;
		float perlin(int3 const &pos) const;

};

class SineTerrainGenerator
:
	public TerrainGenerator
{

	public:

		virtual void generateChunk(ChunkData &data, int3 const &pos) const;

};

class AsyncTerrainGenerator
:
	boost::noncopyable
{

	TerrainGenerator &terrainGenerator;
	ThreadPool threadPool;

	public:

		AsyncTerrainGenerator(TerrainGenerator &terrainGenerator);

		void generate(Chunk *chunk);
		void gather();

	private:

		void work(Chunk *chunk);
		void finalize(Chunk *chunk);

};

#endif
