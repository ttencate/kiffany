#ifndef CHUNK_H
#define CHUNK_H

#include "gl.h"
#include "maths.h"

#include <boost/noncopyable.hpp>

extern const unsigned CHUNK_SIZE;

class Chunk
:
	boost::noncopyable
{

	int3 const pos;

	GLBuffer vertexBuffer;
	GLBuffer normalBuffer;

	public:

		Chunk(int3 const &pos);

		void render() const;

};

#endif
