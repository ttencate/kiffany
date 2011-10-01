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
	std::vector<float> vertexData;
	std::vector<float> normalData;

	public:

		std::vector<float> const &getVertexData() const { return vertexData; }
		std::vector<float> &getVertexData() { return vertexData; }
		std::vector<float> const &getNormalData() const { return normalData; }
		std::vector<float> &getNormalData() { return normalData; }

		bool isEmpty() const;

};

#endif
