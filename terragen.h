#ifndef TERRAGEN_H
#define TERRAGEN_H

#include "chunk.h"
#include "maths.h"
#include "threadpool.h"

#include <boost/noncopyable.hpp>
#include <boost/scoped_array.hpp>
#include <boost/unordered_set.hpp>

class ChunkData;
class ChunkGeometry;

// Must be thread-safe.
class TerrainGenerator {

	public:

		void generateChunk(int3 const &pos, ChunkData *data) const;

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

class AsyncTerrainGenerator
:
	boost::noncopyable
{

	TerrainGenerator &terrainGenerator;
	boost::unordered_set<ChunkPtr> inProgress;
	ThreadPool threadPool; // Must be after everything that it uses!

	public:

		AsyncTerrainGenerator(TerrainGenerator &terrainGenerator);

		bool tryGenerate(ChunkPtr chunk);
		void gather();

	private:

		void work(int3 position, ChunkGeometry* chunkGeometry);
		void finalize(ChunkPtr chunk, ChunkGeometry* chunkGeometry);

};

#endif
