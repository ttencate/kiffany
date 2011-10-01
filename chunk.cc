#include "chunk.h"

#include "stats.h"
#include "terragen.h"

Chunk::Chunk(int3 const &index)
:
	index(index),
	position(CHUNK_SIZE * index.x, CHUNK_SIZE * index.y, CHUNK_SIZE * index.z)
{
}

void Chunk::setData(ChunkData *data) {
	this->data.reset(data);
}

void Chunk::setGeometry(ChunkGeometry *geometry) {
	this->geometry.reset(geometry);
}

bool Chunk::canRender() const {
	return geometry || buffers;
}

void Chunk::render() {
	if (geometry && !buffers) {
		if (!geometry->isEmpty()) {
			buffers.reset(new ChunkBuffers());
			upload(*geometry, buffers.get());
			geometry.reset();
		}
	}
	if (!buffers) {
		return;
	}

	::render(*buffers);
}

void tesselate(ChunkData const &data, int3 const &position, ChunkGeometry *geometry) {
	Timed t = stats.chunkTesselationTime.timed();

	std::vector<float> &vertices = geometry->getVertexData();
	std::vector<float> &normals = geometry->getNormalData();
	vertices.clear();
	normals.clear();

	unsigned s = 0;
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
						vertices.resize(s + 12);
						normals.resize(s + 12);
						memcpy(&vertices[s], v, 12 * sizeof(float));
						memcpy(&normals[s], n, 12 * sizeof(float));
						s += 12;
					}
					if (x == CHUNK_SIZE - 1 || needsDrawing(block, p[1])) {
						float v[] = { M.x, m.y, m.z, M.x, M.y, m.z, M.x, M.y, M.z, M.x, m.y, M.z };
						float n[] = { 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0 };
						vertices.resize(s + 12);
						normals.resize(s + 12);
						memcpy(&vertices[s], v, 12 * sizeof(float));
						memcpy(&normals[s], n, 12 * sizeof(float));
						s += 12;
					}
					if (y == 0 || needsDrawing(block, p[-(int)CHUNK_SIZE])) {
						float v[] = { m.x, m.y, m.z, M.x, m.y, m.z, M.x, m.y, M.z, m.x, m.y, M.z };
						float n[] = { 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0 };
						vertices.resize(s + 12);
						normals.resize(s + 12);
						memcpy(&vertices[s], v, 12 * sizeof(float));
						memcpy(&normals[s], n, 12 * sizeof(float));
						s += 12;
					}
					if (y == CHUNK_SIZE - 1 || needsDrawing(block, p[CHUNK_SIZE])) {
						float v[] = { m.x, M.y, m.z, m.x, M.y, M.z, M.x, M.y, M.z, M.x, M.y, m.z };
						float n[] = { 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0 };
						vertices.resize(s + 12);
						normals.resize(s + 12);
						memcpy(&vertices[s], v, 12 * sizeof(float));
						memcpy(&normals[s], n, 12 * sizeof(float));
						s += 12;
					}
					if (z == 0 || needsDrawing(block, p[-(int)CHUNK_SIZE * (int)CHUNK_SIZE])) {
						float v[] = { m.x, m.y, m.z, m.x, M.y, m.z, M.x, M.y, m.z, M.x, m.y, m.z };
						float n[] = { 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1 };
						vertices.resize(s + 12);
						normals.resize(s + 12);
						memcpy(&vertices[s], v, 12 * sizeof(float));
						memcpy(&normals[s], n, 12 * sizeof(float));
						s += 12;
					}
					if (z == CHUNK_SIZE - 1 || needsDrawing(block, p[CHUNK_SIZE * CHUNK_SIZE])) {
						float v[] = { m.x, m.y, M.z, M.x, m.y, M.z, M.x, M.y, M.z, m.x, M.y, M.z };
						float n[] = { 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1 };
						vertices.resize(s + 12);
						normals.resize(s + 12);
						memcpy(&vertices[s], v, 12 * sizeof(float));
						memcpy(&normals[s], n, 12 * sizeof(float));
						s += 12;
					}
				}
				++p;
			}
		}
	}

	stats.chunksTesselated.increment();
	stats.quadsGenerated.increment(vertices.size() / 4);
}

void upload(ChunkGeometry const &geometry, ChunkBuffers *buffers) {
	buffers->getVertexBuffer().putData(
			geometry.getVertexData().size() * sizeof(float),
			&(geometry.getVertexData()[0]),
			GL_STATIC_DRAW);
	buffers->getNormalBuffer().putData(
			geometry.getNormalData().size() * sizeof(float),
			&(geometry.getNormalData()[0]),
			GL_STATIC_DRAW);
}

void render(ChunkBuffers const &buffers) {
	glEnableClientState(GL_VERTEX_ARRAY);
	glBindBuffer(GL_ARRAY_BUFFER, buffers.getVertexBuffer().getName());
	glVertexPointer(3, GL_FLOAT, 0, 0);

	glEnableClientState(GL_NORMAL_ARRAY);
	glBindBuffer(GL_ARRAY_BUFFER, buffers.getNormalBuffer().getName());
	glNormalPointer(GL_FLOAT, 0, 0);

	glDrawArrays(GL_QUADS, 0, buffers.getVertexBuffer().getSizeInBytes() / sizeof(float) / 3);
	stats.quadsRendered.increment(buffers.getVertexBuffer().getSizeInBytes() / sizeof(float) / 3 / 4);
	stats.chunksRendered.increment();
}
