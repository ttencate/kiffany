#include "chunk.h"

#include "stats.h"
#include "terragen.h"

bool ChunkGeometry::isEmpty() const {
	return vertexData.size() == 0;
}

void upload(ChunkGeometry const &geometry, ChunkBuffers *buffers) {
	buffers->getVertexBuffer().putData(
			geometry.getVertexData().size() * sizeof(short),
			&(geometry.getVertexData()[0]),
			GL_STATIC_DRAW);
	buffers->getNormalBuffer().putData(
			geometry.getNormalData().size() * sizeof(char),
			&(geometry.getNormalData()[0]),
			GL_STATIC_DRAW);
}

void render(ChunkBuffers const &buffers) {
	glEnableClientState(GL_VERTEX_ARRAY);
	glBindBuffer(GL_ARRAY_BUFFER, buffers.getVertexBuffer().getName());
	glVertexPointer(3, GL_SHORT, 0, 0);

	glEnableClientState(GL_NORMAL_ARRAY);
	glBindBuffer(GL_ARRAY_BUFFER, buffers.getNormalBuffer().getName());
	glNormalPointer(GL_BYTE, 0, 0);

	glDrawArrays(GL_QUADS, 0, buffers.getVertexBuffer().getSizeInBytes() / sizeof(short) / 3);
	stats.quadsRendered.increment(buffers.getVertexBuffer().getSizeInBytes() / sizeof(short) / 3 / 4);
	stats.chunksRendered.increment();
}

Chunk::Chunk(int3 const &index)
:
	index(index),
	position(CHUNK_SIZE * index.x, CHUNK_SIZE * index.y, CHUNK_SIZE * index.z)
{
}

void Chunk::setData(ChunkData *data) {
	this->data.reset(data);
}

bool Chunk::canRender() const {
	if (data) {
		return true;
	}
	for (unsigned i = 0; i < 6; ++i) {
		if (geometry[i] || buffers[i]) {
			return true;
		}
	}
	return false;
}

void Chunk::render() {
	if (data) {
		tesselate();
	}
	for (unsigned i = 0; i < 6; ++i) {
		if (geometry[i] && !buffers[i]) {
			if (!geometry[i]->isEmpty()) {
				buffers[i].reset(new ChunkBuffers());
				upload(*geometry[i], buffers[i].get());
				geometry[i].reset();
			}
		}
		if (buffers[i]) {
			::render(*buffers[i]);
		}
	}
}

template<int dx, int dy, int dz>
void tesselate(ChunkData const &data, int3 const &position, ChunkGeometry *geometry) {
	char const N = 0x7F;
	char const n[12] = {
		dx * N, dy * N, dz * N,
		dx * N, dy * N, dz * N,
		dx * N, dy * N, dz * N,
		dx * N, dy * N, dz * N,
	};
	int const neighOffset = dx + (int)CHUNK_SIZE * dy + (int)CHUNK_SIZE * (int)CHUNK_SIZE * dz;

	Timed t = stats.chunkTesselationTime.timed();

	std::vector<short> &vertices = geometry->getVertexData();
	std::vector<char> &normals = geometry->getNormalData();
	vertices.clear();
	normals.clear();

	unsigned s = 0;
	Block const *p = data.raw() + 1;
	for (unsigned z = 1; z < CHUNK_SIZE - 1; ++z) {
		for (unsigned y = 1; y < CHUNK_SIZE - 1; ++y) {
			for (unsigned x = 1; x < CHUNK_SIZE - 1; ++x) {
				Block const block = *p;
				if (needsDrawing(block) && needsDrawing(block, p[neighOffset])) {
					int3 const pos(x, y, z);
					int3 m = (int3)blockMin(position + pos); // TODO make relative, use matrix
					int3 M = (int3)blockMax(position + pos);
					short v[] = { m.x, m.y, m.z, m.x, m.y, M.z, m.x, M.y, M.z, m.x, M.y, m.z };
					vertices.resize(s + 12);
					normals.resize(s + 12);
					memcpy(&vertices[s], v, 12 * sizeof(short));
					memcpy(&normals[s], n, 12 * sizeof(char));
					s += 12;
				}
				++p;
			}
			p += 2;
		}
		p += 2 * CHUNK_SIZE;
	}

	stats.chunksTesselated.increment();
	stats.quadsGenerated.increment(vertices.size() / 4);
}

void Chunk::tesselate() {
	for (unsigned i = 0; i < 6; ++i) {
		geometry[i].reset(new ChunkGeometry());
	}
	::tesselate<-1,  0,  0>(*data, position, geometry[0].get());
	::tesselate< 1,  0,  0>(*data, position, geometry[1].get());
	::tesselate< 0, -1,  0>(*data, position, geometry[2].get());
	::tesselate< 0,  1,  0>(*data, position, geometry[3].get());
	::tesselate< 0,  0, -1>(*data, position, geometry[4].get());
	::tesselate< 0,  0,  1>(*data, position, geometry[5].get());

	data.reset(); // No more use for this.
}
