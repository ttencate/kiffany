#ifndef TERRAGEN_H
#define TERRAGEN_H

#include "maths.h"
#include "threadpool.h"

#include <boost/noncopyable.hpp>
#include <boost/scoped_array.hpp>

class Chunk;
class ChunkData;
class ChunkGeometry;

class TerrainGenerator {

	public:

		virtual void generateChunk(int3 const &pos, ChunkData *data) const = 0;

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

		virtual void generateChunk(int3 const &pos, ChunkData *data) const;

	private:

		float lookup(int x, int y, int z) const;
		float perlin(int3 const &pos) const;

};

class SineTerrainGenerator
:
	public TerrainGenerator
{

	public:

		virtual void generateChunk(int3 const &pos, ChunkData *data) const;

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

		void work(int3 position, ChunkGeometry *chunkGeometry);
		void finalize(Chunk *chunk, ChunkGeometry *chunkGeometry);

};

#endif
