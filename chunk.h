#ifndef CHUNK_H
#define CHUNK_H

#include "buffer.h"
#include "maths.h"

#include <boost/noncopyable.hpp>

extern const unsigned CHUNK_SIZE;

class Chunk
:
	boost::noncopyable
{

	int3 const pos;

	Buffer vertexBuffer;
	Buffer normalBuffer;

	public:

		Chunk(int3 const &pos);

		void render() const;

};

#endif
