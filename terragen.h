#ifndef TERRAGEN_H
#define TERRAGEN_H

#include "maths.h"
#include "threadpool.h"

#include <boost/noncopyable.hpp>
#include <boost/scoped_array.hpp>

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

	// Must be binary serializable
	struct WorkUnit {
		TerrainGenerator *terrainGenerator;
		ChunkData *chunkData;
		int3 pos;
	};

	TerrainGenerator &terrainGenerator;
	ThreadPool<WorkUnit> threadPool;

	public:

		AsyncTerrainGenerator(TerrainGenerator &terrainGenerator);

		void sow(int3 const &pos, ChunkData *chunkData);
		bool reap(int3 *pos, ChunkData **chunkData);

	private:

		static void work(WorkUnit &workUnit);

};

#endif
