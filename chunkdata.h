#ifndef CHUNKDATA_H
#define CHUNKDATA_H

#include "block.h"
#include "maths.h"

#include <boost/noncopyable.hpp>
#include <boost/scoped_array.hpp>

extern const unsigned CHUNK_SIZE;

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

		Block &operator[](int3 pos);
		Block const &operator[](int3 pos) const;

		iterator begin();
		iterator end();
		const_iterator begin() const;
		const_iterator end() const;

		coords_iterator beginCoords() const;
		coords_iterator endCoords() const;

};

vec3 blockMin(int3 const &pos);
vec3 blockMax(int3 const &pos);
vec3 blockCenter(int3 const &pos);

#endif
