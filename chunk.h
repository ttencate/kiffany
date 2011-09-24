#ifndef CHUNK_H
#define CHUNK_H

#include "block.h"
#include "gl.h"
#include "maths.h"

#include <boost/noncopyable.hpp>
#include <boost/scoped_array.hpp>

extern const unsigned CHUNK_SIZE;

class TerrainGenerator;

class ChunkData
:
	boost::noncopyable
{

	class CoordsIterator {

		int3 coords;

		public:

			CoordsIterator(int3 const &coords);

			CoordsIterator &operator++();
			CoordsIterator operator++(int);

			bool operator==(CoordsIterator const &other) const;
			bool operator!=(CoordsIterator const &other) const;

			int3 const &operator*() const;

	};

	boost::scoped_array<Block> blocks;

	public:

		typedef Block* iterator;
		typedef Block const* const_iterator;
		typedef CoordsIterator coords_iterator;

		ChunkData();

		Block &at(int3 pos);
		Block const &at(int3 pos) const;

		iterator begin();
		iterator end();
		const_iterator begin() const;
		const_iterator end() const;

		coords_iterator beginCoords() const;
		coords_iterator endCoords() const;

};

class Chunk
:
	boost::noncopyable
{

	int3 const pos;

	ChunkData data;

	// TODO should be in separate class, make Chunk gl-independent
	GLBuffer vertexBuffer;
	GLBuffer normalBuffer;

	public:

		Chunk(int3 const &pos, TerrainGenerator &terrainGenerator);

		void render() const;

};

#endif
