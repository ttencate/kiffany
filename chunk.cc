#include "chunk.h"

#include <GL/glew.h>

const unsigned CHUNK_SIZE = 32;

Chunk::Chunk(glm::int3 const &pos)
:
	pos(pos)
{
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
	float const d = CHUNK_SIZE;
	float const x = d * pos.x, y = d * pos.y, z = d * pos.z;
	float const X = x + d, Y = y + d, Z = z + d;
	float vertices[] = {
		x, y, z,
		x, y, Z,
		x, Y, Z,
		x, Y, z,

		X, y, z,
		X, Y, z,
		X, Y, Z,
		X, y, Z,

		x, y, z,
		X, y, z,
		X, y, Z,
		x, y, Z,

		x, Y, z,
		x, Y, Z,
		X, Y, Z,
		X, Y, z,

		x, y, z,
		x, Y, z,
		X, Y, z,
		X, y, z,

		x, y, Z,
		X, y, Z,
		X, Y, Z,
		x, Y, Z,
	};
	glBufferData(GL_ARRAY_BUFFER, 6 * 4 * 3 * sizeof(float), vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, normalBuffer);
	float normals[] = {
		-1, 0, 0,
		-1, 0, 0,
		-1, 0, 0,
		-1, 0, 0,

		1, 0, 0,
		1, 0, 0,
		1, 0, 0,
		1, 0, 0,

		0, -1, 0,
		0, -1, 0,
		0, -1, 0,
		0, -1, 0,

		0, 1, 0,
		0, 1, 0,
		0, 1, 0,
		0, 1, 0,

		0, 0, -1,
		0, 0, -1,
		0, 0, -1,
		0, 0, -1,

		0, 0, 1,
		0, 0, 1,
		0, 0, 1,
		0, 0, 1,
	};
	glBufferData(GL_ARRAY_BUFFER, 6 * 4 * 3 * sizeof(float), normals, GL_STATIC_DRAW);
}

void Chunk::render() const {
	glEnableClientState(GL_VERTEX_ARRAY);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
	glVertexPointer(3, GL_FLOAT, 0, 0);

	glEnableClientState(GL_NORMAL_ARRAY);
	glBindBuffer(GL_ARRAY_BUFFER, normalBuffer);
	glNormalPointer(GL_FLOAT, 0, 0);

	glDrawArrays(GL_QUADS, 0, 6 * 4);
}
