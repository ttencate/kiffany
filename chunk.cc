#include "chunk.h"

#include "stats.h"
#include "terragen.h"

void upload(ChunkGeometry const &geometry, ChunkBuffers *buffers) {
	buffers->getVertexBuffer().putData(
			geometry.getVertexData().size() * sizeof(short),
			&(geometry.getVertexData()[0]),
			GL_STATIC_DRAW);
	buffers->getNormalBuffer().putData(
			geometry.getNormalData().size() * sizeof(char),
			&(geometry.getNormalData()[0]),
			GL_STATIC_DRAW);
	buffers->setRanges(geometry.getRanges());
}

void render(ChunkBuffers const &buffers) {
	bindBuffer(GL_ARRAY_BUFFER, buffers.getVertexBuffer());
	glVertexPointer(3, GL_SHORT, 0, 0);

	bindBuffer(GL_ARRAY_BUFFER, buffers.getNormalBuffer());
	glNormalPointer(GL_BYTE, 0, 0);

	glDrawArrays(GL_QUADS, 0, buffers.getVertexBuffer().getSizeInBytes() / sizeof(short) / 3);
	stats.quadsRendered.increment(buffers.getVertexBuffer().getSizeInBytes() / sizeof(short) / 3 / 4);
}

Chunk::Chunk(int3 const &index)
:
	index(index),
	position(CHUNK_SIZE * index.x, CHUNK_SIZE * index.y, CHUNK_SIZE * index.z),
	state(NEW),
	upgrading(false)
{
}

void Chunk::startUpgrade() {
	upgrading = true;
}

void Chunk::endUpgrade() {
	upgrading = false;
	BOOST_ASSERT(state < LIGHTED);
	state = (State)(state + 1);
}

void Chunk::setOctree(OctreePtr octree) {
	this->octree = octree;
}

void Chunk::setGeometry(ChunkGeometryPtr geometry) {
	this->geometry = geometry;
	buffers.reset();
}

void Chunk::render() {
	if (!geometry || geometry->isEmpty()) {
		return;
	}
	if (!buffers) {
		buffers.reset(new ChunkBuffers());
		::upload(*geometry, buffers.get());
	}
	if (buffers) {
		glPushMatrix();
		glTranslated(position.x, position.y, position.z);
		::render(*buffers);
		glPopMatrix();
		stats.chunksRendered.increment();
	} else {
		stats.chunksEmpty.increment();
	}
}
