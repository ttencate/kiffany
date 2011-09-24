#ifndef TERRAGEN_H
#define TERRAGEN_H

#include "maths.h"

class ChunkData;

class TerrainGenerator {

	public:

		virtual void generateChunk(ChunkData &data, int3 const &pos) const = 0;

};

class SineTerrainGenerator
:
	public TerrainGenerator
{

	public:

		virtual void generateChunk(ChunkData &data, int3 const &pos) const;

};

#endif
