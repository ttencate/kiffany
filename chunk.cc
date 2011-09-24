#include "chunk.h"

#include "terragen.h"

#include <algorithm>
#include <iterator>
#include <vector>

void Chunk::fillBuffers() {
	float const cubeNormals[] = {
		-1,  0,  0, -1,  0,  0, -1,  0,  0, -1,  0,  0,
		 1,  0,  0,  1,  0,  0,  1,  0,  0,  1,  0,  0,
		 0, -1,  0,  0, -1,  0,  0, -1,  0,  0, -1,  0,
		 0,  1,  0,  0,  1,  0,  0,  1,  0,  0,  1,  0,
		 0,  0, -1,  0,  0, -1,  0,  0, -1,  0,  0, -1,
		 0,  0,  1,  0,  0,  1,  0,  0,  1,  0,  0,  1,
	};
	std::vector<float> vertices;
	std::vector<float> normals;
	for (ChunkData::coords_iterator i = data.beginCoords(); i != data.endCoords(); ++i) {
		if (data[*i] == STONE_BLOCK) {
			vec3 m = blockMin(position + *i);
			vec3 M = blockMax(position + *i);
			float const x = m.x, y = m.y, z = m.z;
			float const X = M.x, Y = M.y, Z = M.z;
			float cubeVertices[] = {
				x, y, z, x, y, Z, x, Y, Z, x, Y, z,
				X, y, z, X, Y, z, X, Y, Z, X, y, Z,
				x, y, z, X, y, z, X, y, Z, x, y, Z,
				x, Y, z, x, Y, Z, X, Y, Z, X, Y, z,
				x, y, z, x, Y, z, X, Y, z, X, y, z,
				x, y, Z, X, y, Z, X, Y, Z, x, Y, Z,
			};
			std::copy(cubeVertices, cubeVertices + 6 * 4 * 3, std::back_inserter(vertices));
			std::copy(cubeNormals, cubeNormals + 6 * 4 * 3, std::back_inserter(normals));
		}
	}
	vertexBuffer.putData(vertices.size() * sizeof(float), &vertices[0], GL_STATIC_DRAW);
	normalBuffer.putData(normals.size() * sizeof(float), &normals[0], GL_STATIC_DRAW);
}

Chunk::Chunk(int3 const &index, TerrainGenerator &terrainGenerator)
:
	index(index),
	position(CHUNK_SIZE * index.x, CHUNK_SIZE * index.y, CHUNK_SIZE * index.z)
{
	terrainGenerator.generateChunk(data, position);
	fillBuffers();
}

void Chunk::render() const {
	glEnableClientState(GL_VERTEX_ARRAY);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
	glVertexPointer(3, GL_FLOAT, 0, 0);

	glEnableClientState(GL_NORMAL_ARRAY);
	glBindBuffer(GL_ARRAY_BUFFER, normalBuffer);
	glNormalPointer(GL_FLOAT, 0, 0);

	glDrawArrays(GL_QUADS, 0, vertexBuffer.getSizeInBytes() / sizeof(float) / 3);
}
