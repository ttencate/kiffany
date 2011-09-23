#ifndef CHUNK_H
#define CHUNK_H

#include "glm/ext.hpp"

#include <boost/noncopyable.hpp>

extern const unsigned CHUNK_SIZE;

class Chunk
:
	boost::noncopyable
{

	glm::int3 const pos;

	unsigned vertexBuffer;
	unsigned normalBuffer;

	public:

		Chunk(glm::int3 const &pos);

		void render() const;

};

#endif
