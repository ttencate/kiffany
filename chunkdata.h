#ifndef CHUNKDATA_H
#define CHUNKDATA_H

#include "block.h"
#include "coords.h"
#include "maths.h"

#include <boost/noncopyable.hpp>
#include <boost/scoped_array.hpp>
#include <boost/shared_ptr.hpp>

#include <vector>

class RawChunkData
:
	boost::noncopyable
{

	boost::scoped_array<Block> blocks;

	public:

		typedef Block* iterator;
		typedef Block const* const_iterator;

		RawChunkData();

		Block &operator[](int3 pos);
		Block const &operator[](int3 pos) const;

		iterator begin();
		iterator end();
		const_iterator begin() const;
		const_iterator end() const;

		Block const *raw() const;
		Block *raw();

};

typedef boost::shared_ptr<RawChunkData> RawChunkDataPtr;

#endif
