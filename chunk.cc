#include "chunk.h"

#include "stats.h"
#include "terragen.h"

#include <algorithm>
#include <iterator>
#include <vector>

Chunk::Chunk(int3 const &index)
:
	index(index),
	position(CHUNK_SIZE * index.x, CHUNK_SIZE * index.y, CHUNK_SIZE * index.z),
	generated(false),
	tesselated(false)
{
}

void Chunk::render() {
	if (!generated) {
		return;
	}
	if (!tesselated) {
		{
			Timed t = stats.chunkTesselationTime.timed();
			tesselate();
		}
		stats.chunksTesselated.increment();
	}
	if (vertexBuffer.isEmpty()) {
		return;
	}

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

void Chunk::tesselate() {
	std::vector<float> vertices;
	std::vector<float> normals;
	Block const *p = data.raw();
	for (unsigned z = 0; z < CHUNK_SIZE; ++z) {
		for (unsigned y = 0; y < CHUNK_SIZE; ++y) {
			for (unsigned x = 0; x < CHUNK_SIZE; ++x) {
				Block const block = *p;
				if (needsDrawing(block)) {
					int3 const pos(x, y, z);
					vec3 m = blockMin(position + pos);
					vec3 M = blockMax(position + pos);
					if (x == 0 || needsDrawing(block, p[-1])) {
						float v[] = { m.x, m.y, m.z, m.x, m.y, M.z, m.x, M.y, M.z, m.x, M.y, m.z };
						float n[] = { -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0 };
						std::copy(v, v + 12, std::back_inserter(vertices));
						std::copy(n, n + 12, std::back_inserter(normals));
					}
					if (x == CHUNK_SIZE - 1 || needsDrawing(block, p[1])) {
						float v[] = { M.x, m.y, m.z, M.x, M.y, m.z, M.x, M.y, M.z, M.x, m.y, M.z };
						float n[] = { 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0 };
						std::copy(v, v + 12, std::back_inserter(vertices));
						std::copy(n, n + 12, std::back_inserter(normals));
					}
					if (y == 0 || needsDrawing(block, p[-(int)CHUNK_SIZE])) {
						float v[] = { m.x, m.y, m.z, M.x, m.y, m.z, M.x, m.y, M.z, m.x, m.y, M.z };
						float n[] = { 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0 };
						std::copy(v, v + 12, std::back_inserter(vertices));
						std::copy(n, n + 12, std::back_inserter(normals));
					}
					if (y == CHUNK_SIZE - 1 || needsDrawing(block, p[CHUNK_SIZE])) {
						float v[] = { m.x, M.y, m.z, m.x, M.y, M.z, M.x, M.y, M.z, M.x, M.y, m.z };
						float n[] = { 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0 };
						std::copy(v, v + 12, std::back_inserter(vertices));
						std::copy(n, n + 12, std::back_inserter(normals));
					}
					if (z == 0 || needsDrawing(block, p[-(int)CHUNK_SIZE * (int)CHUNK_SIZE])) {
						float v[] = { m.x, m.y, m.z, m.x, M.y, m.z, M.x, M.y, m.z, M.x, m.y, m.z };
						float n[] = { 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1 };
						std::copy(v, v + 12, std::back_inserter(vertices));
						std::copy(n, n + 12, std::back_inserter(normals));
					}
					if (z == CHUNK_SIZE - 1 || needsDrawing(block, p[CHUNK_SIZE * CHUNK_SIZE])) {
						float v[] = { m.x, m.y, M.z, M.x, m.y, M.z, M.x, M.y, M.z, m.x, M.y, M.z };
						float n[] = { 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1 };
						std::copy(v, v + 12, std::back_inserter(vertices));
						std::copy(n, n + 12, std::back_inserter(normals));
					}
				}
				++p;
			}
		}
	}
	vertexBuffer.putData(vertices.size() * sizeof(float), &vertices[0], GL_STATIC_DRAW);
	normalBuffer.putData(normals.size() * sizeof(float), &normals[0], GL_STATIC_DRAW);

	stats.quadsGenerated.increment(vertices.size() / 4);
	tesselated = true;
}
