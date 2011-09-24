#ifndef CHUNK_H
#define CHUNK_H

#include "block.h"
#include "gl.h"
#include "maths.h"

#include <boost/noncopyable.hpp>
#include <boost/scoped_array.hpp>

extern const unsigned CHUNK_SIZE;

class ChunkData
:
	boost::noncopyable
{

	boost::scoped_array<Block> blocks;

	public:

		typedef Block* iterator;
		typedef Block const* const_iterator;

		ChunkData();

		Block &at(int3 pos);
		Block const &at(int3 pos) const;

		iterator begin();
		iterator end();
		const_iterator begin() const;
		const_iterator end() const;

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

		Chunk(int3 const &pos);

		void render() const;

};

#endif
