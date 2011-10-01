#ifndef CHUNKDATA_H
#define CHUNKDATA_H

#include "block.h"
#include "coords.h"
#include "maths.h"

#include <boost/noncopyable.hpp>
#include <boost/scoped_array.hpp>

#include <vector>

class ChunkData
:
	boost::noncopyable
{

	boost::scoped_array<Block> blocks;

	public:

		typedef Block* iterator;
		typedef Block const* const_iterator;

		ChunkData();

		Block &operator[](int3 pos);
		Block const &operator[](int3 pos) const;

		iterator begin();
		iterator end();
		const_iterator begin() const;
		const_iterator end() const;

		CoordsBlock getCoordsBlock() const;

		Block const *raw() const;
		Block *raw();

};

class ChunkGeometry
:
	boost::noncopyable
{
	std::vector<short> vertexData;
	std::vector<char> normalData;

	public:

		std::vector<short> const &getVertexData() const { return vertexData; }
		std::vector<short> &getVertexData() { return vertexData; }
		std::vector<char> const &getNormalData() const { return normalData; }
		std::vector<char> &getNormalData() { return normalData; }

		bool isEmpty() const;

};

void tesselate(ChunkData const &data, int3 const &position, ChunkGeometry *geometry);

#endif
