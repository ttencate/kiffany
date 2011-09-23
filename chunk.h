#ifndef CHUNK_H
#define CHUNK_H

#include "buffer.h"

#include "glm/ext.hpp"

#include <boost/noncopyable.hpp>

extern const unsigned CHUNK_SIZE;

class Chunk
:
	boost::noncopyable
{

	glm::int3 const pos;

	Buffer vertexBuffer;
	Buffer normalBuffer;

	public:

		Chunk(glm::int3 const &pos);

		void render() const;

};

#endif
