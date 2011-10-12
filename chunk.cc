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
	position(CHUNK_SIZE * index.x, CHUNK_SIZE * index.y, CHUNK_SIZE * index.z),
	state(NEW)
{
}

void Chunk::setData(ChunkData *data) {
	this->data.reset(data);
	state = data ? GENERATED : NEW;
}

bool Chunk::needsGenerating() const {
	return state == NEW;
}

void Chunk::render() {
	if (state < GENERATED) {
		return;
	}
	if (state == GENERATED) {
		tesselate();
	}
	if (state == TESSELATED) {
		upload();
	}
	if (state == UPLOADED) {
		for (unsigned i = 0; i < 6; ++i) {
			if (buffers[i]) {
				::render(*buffers[i]);
			}
		}
	}
}

template<int dx, int dy, int dz, int faceIndex>
void tesselateFace(ChunkData const &data, int3 const &position, ChunkGeometry *geometry) {
	static char const N = 0x7F;
	static char const n[12] = {
		dx * N, dy * N, dz * N,
		dx * N, dy * N, dz * N,
		dx * N, dy * N, dz * N,
		dx * N, dy * N, dz * N,
	};
	static short const faces[6][12] = {
		{ 0, 0, 0, 0, 0, 1, 0, 1, 1, 0, 1, 0 },
		{ 1, 0, 0, 1, 1, 0, 1, 1, 1, 1, 0, 1 },
		{ 0, 0, 0, 1, 0, 0, 1, 0, 1, 0, 0, 1 },
		{ 0, 1, 0, 0, 1, 1, 1, 1, 1, 1, 1, 0 },
		{ 0, 0, 0, 0, 1, 0, 1, 1, 0, 1, 0, 0 },
		{ 0, 0, 1, 1, 0, 1, 1, 1, 1, 0, 1, 1 }
	};
	static short const *face = faces[faceIndex];
	static int const neighOffset = dx + (int)CHUNK_SIZE * dy + (int)CHUNK_SIZE * (int)CHUNK_SIZE * dz;

	std::vector<short> &vertices = geometry->getVertexData();
	std::vector<char> &normals = geometry->getNormalData();
	vertices.clear();
	normals.clear();

	unsigned s = 0;
	Block const *p = data.raw() + CHUNK_SIZE * CHUNK_SIZE + CHUNK_SIZE + 1;
	for (unsigned z = 1; z < CHUNK_SIZE - 1; ++z) {
		for (unsigned y = 1; y < CHUNK_SIZE - 1; ++y) {
			for (unsigned x = 1; x < CHUNK_SIZE - 1; ++x) {
				Block const block = *p;
				if (needsDrawing(block) && needsDrawing(block, p[neighOffset])) {
					int3 const pos(x, y, z);
					int3 m = (int3)blockMin(position + pos); // TODO make relative, use matrix
					short v[12] = {
						face[0] + m.x, face[ 1] + m.y, face[ 2] + m.z,
						face[3] + m.x, face[ 4] + m.y, face[ 5] + m.z,
						face[6] + m.x, face[ 7] + m.y, face[ 8] + m.z,
						face[9] + m.x, face[10] + m.y, face[11] + m.z
					};
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

	Timed t = stats.chunkTesselationTime.timed();

	tesselateFace<-1,  0,  0, 0>(*data, position, geometry[0].get());
	tesselateFace< 1,  0,  0, 1>(*data, position, geometry[1].get());
	tesselateFace< 0, -1,  0, 2>(*data, position, geometry[2].get());
	tesselateFace< 0,  1,  0, 3>(*data, position, geometry[3].get());
	tesselateFace< 0,  0, -1, 4>(*data, position, geometry[4].get());
	tesselateFace< 0,  0,  1, 5>(*data, position, geometry[5].get());

	data.reset(); // No more use for this.
	state = TESSELATED;
}

void Chunk::upload() { 
	for (unsigned i = 0; i < 6; ++i) {
		if (geometry[i]) {
			if (!geometry[i]->isEmpty()) {
				buffers[i].reset(new ChunkBuffers());
				::upload(*geometry[i], buffers[i].get());
			}
			geometry[i].reset();
		}
	}
	state = UPLOADED;
}
