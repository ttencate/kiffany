#include "chunk.h"

#include "stats.h"
#include "terragen.h"

#include <algorithm>
#include <iterator>
#include <vector>

void Chunk::fillBuffers() {
	std::vector<float> vertices;
	std::vector<float> normals;
	CoordsBlock coordsBlock = data.getCoordsBlock();
	for (CoordsBlock::const_iterator i = coordsBlock.begin(); i != coordsBlock.end(); ++i) {
		int3 const pos = *i;
		Block block = data[pos];
		if (!needsDrawing(block)) {
			continue;
		}
		vec3 m = blockMin(position + pos);
		vec3 M = blockMax(position + pos);
		float const x = m.x, y = m.y, z = m.z;
		float const X = M.x, Y = M.y, Z = M.z;
		if (pos.x == 0 || needsDrawing(block, data[pos - X_STEP])) {
			float v[] = { x, y, z, x, y, Z, x, Y, Z, x, Y, z };
			float n[] = { -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, };
			std::copy(v, v + 12, std::back_inserter(vertices));
			std::copy(n, n + 12, std::back_inserter(normals));
		}
		if (pos.x == CHUNK_SIZE - 1 || needsDrawing(block, data[pos + X_STEP])) {
			float v[] = { X, y, z, X, Y, z, X, Y, Z, X, y, Z, };
			float n[] = { 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, };
			std::copy(v, v + 12, std::back_inserter(vertices));
			std::copy(n, n + 12, std::back_inserter(normals));
		}
		if (pos.y == 0 || needsDrawing(block, data[pos - Y_STEP])) {
			float v[] = { x, y, z, X, y, z, X, y, Z, x, y, Z, };
			float n[] = { 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, };
			std::copy(v, v + 12, std::back_inserter(vertices));
			std::copy(n, n + 12, std::back_inserter(normals));
		}
		if (pos.y == CHUNK_SIZE - 1 || needsDrawing(block, data[pos + Y_STEP])) {
			float v[] = { x, Y, z, x, Y, Z, X, Y, Z, X, Y, z, };
			float n[] = { 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, };
			std::copy(v, v + 12, std::back_inserter(vertices));
			std::copy(n, n + 12, std::back_inserter(normals));
		}
		if (pos.z == 0 || needsDrawing(block, data[pos - Z_STEP])) {
			float v[] = { x, y, z, x, Y, z, X, Y, z, X, y, z, };
			float n[] = { 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, };
			std::copy(v, v + 12, std::back_inserter(vertices));
			std::copy(n, n + 12, std::back_inserter(normals));
		}
		if (pos.z == 0 || needsDrawing(block, data[pos + Z_STEP])) {
			float v[] = { x, y, Z, X, y, Z, X, Y, Z, x, Y, Z, };
			float n[] = { 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, };
			std::copy(v, v + 12, std::back_inserter(vertices));
			std::copy(n, n + 12, std::back_inserter(normals));
		}
	}
	vertexBuffer.putData(vertices.size() * sizeof(float), &vertices[0], GL_STATIC_DRAW);
	normalBuffer.putData(normals.size() * sizeof(float), &normals[0], GL_STATIC_DRAW);
	stats.quadsGenerated.increment(vertices.size() / 4);
}

Chunk::Chunk(int3 const &index, TerrainGenerator &terrainGenerator)
:
	index(index),
	position(CHUNK_SIZE * index.x, CHUNK_SIZE * index.y, CHUNK_SIZE * index.z)
{
	stats.chunksGenerated.increment();
	terrainGenerator.generateChunk(data, position);
	fillBuffers();
}

void Chunk::render() const {
	stats.chunksRendered.increment();

	glEnableClientState(GL_VERTEX_ARRAY);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
	glVertexPointer(3, GL_FLOAT, 0, 0);

	glEnableClientState(GL_NORMAL_ARRAY);
	glBindBuffer(GL_ARRAY_BUFFER, normalBuffer);
	glNormalPointer(GL_FLOAT, 0, 0);

	glDrawArrays(GL_QUADS, 0, vertexBuffer.getSizeInBytes() / sizeof(float) / 3);
	stats.quadsRendered.increment(vertexBuffer.getSizeInBytes() / sizeof(float) / 3 / 4);
}
