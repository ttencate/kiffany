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

void upload(ChunkGeometry const &geometry, ChunkBuffers *buffers) {
	buffers->getVertexBuffer().putData(
			geometry.getVertexData().size() * sizeof(short),
			&(geometry.getVertexData()[0]),
			GL_STATIC_DRAW);
	buffers->getNormalBuffer().putData(
			geometry.getNormalData().size() * sizeof(short),
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
