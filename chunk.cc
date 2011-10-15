#include "chunk.h"

#include "stats.h"
#include "terragen.h"

template<int dx, int dy, int dz>
struct FaceIndex {
	static int value;
};

template<> int FaceIndex<-1,  0,  0>::value = 0;
template<> int FaceIndex< 1,  0,  0>::value = 1;
template<> int FaceIndex< 0, -1,  0>::value = 2;
template<> int FaceIndex< 0,  1,  0>::value = 3;
template<> int FaceIndex< 0,  0, -1>::value = 4;
template<> int FaceIndex< 0,  0,  1>::value = 5;

template<int dx, int dy, int dz>
void tess(
		Block block, Block neigh,
		unsigned x, unsigned y, unsigned z,
		int3 const &position, // TODO remove once relative
		VertexArray &vertices, NormalArray &normals, unsigned &end)
{
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
	static short const *face = faces[FaceIndex<dx, dy, dz>::value];

	if (needsDrawing(block) && needsDrawing(block, neigh)) {
		int3 const pos(x, y, z);
		int3 m = (int3)blockMin(position + pos); // TODO make relative, use matrix
		short v[12] = {
			face[0] + m.x, face[ 1] + m.y, face[ 2] + m.z,
			face[3] + m.x, face[ 4] + m.y, face[ 5] + m.z,
			face[6] + m.x, face[ 7] + m.y, face[ 8] + m.z,
			face[9] + m.x, face[10] + m.y, face[11] + m.z
		};
		vertices.resize(end + 12);
		normals.resize(end + 12);
		memcpy(&vertices[end], v, 12 * sizeof(short));
		memcpy(&normals[end], n, 12 * sizeof(char));
		end += 12;
	}
}

template<int dx, int dy, int dz>
void tesselateNeigh(Block const *rawData, Block const *rawNeighData, int3 const &position, VertexArray &vertices, NormalArray &normals, unsigned &end);

template<>
void tesselateNeigh<-1, 0, 0>(Block const *p, Block const *q, int3 const &position, VertexArray &vertices, NormalArray &normals, unsigned &end) {
	q += CHUNK_SIZE - 1;
	for (unsigned z = 0; z < CHUNK_SIZE; ++z) {
		for (unsigned y = 0; y < CHUNK_SIZE; ++y) {
			tess<-1, 0, 0>(*p, *q, (unsigned)0, y, z, position, vertices, normals, end);
			p += CHUNK_SIZE;
			q += CHUNK_SIZE;
		}
	}
}

template<>
void tesselateNeigh<1, 0, 0>(Block const *p, Block const *q, int3 const &position, VertexArray &vertices, NormalArray &normals, unsigned &end) {
	p += CHUNK_SIZE - 1;
	for (unsigned z = 0; z < CHUNK_SIZE; ++z) {
		for (unsigned y = 0; y < CHUNK_SIZE; ++y) {
			tess<1, 0, 0>(*p, *q, CHUNK_SIZE - 1, y, z, position, vertices, normals, end);
			p += CHUNK_SIZE;
			q += CHUNK_SIZE;
		}
	}
}

template<>
void tesselateNeigh<0, -1, 0>(Block const *p, Block const *q, int3 const &position, VertexArray &vertices, NormalArray &normals, unsigned &end) {
	q += CHUNK_SIZE * (CHUNK_SIZE - 1);
	for (unsigned z = 0; z < CHUNK_SIZE; ++z) {
		for (unsigned x = 0; x < CHUNK_SIZE; ++x) {
			tess<0, -1, 0>(*p, *q, x, (unsigned)0, z, position, vertices, normals, end);
			++p;
			++q;
		}
		p += CHUNK_SIZE * (CHUNK_SIZE - 1);
		q += CHUNK_SIZE * (CHUNK_SIZE - 1);
	}
}

template<>
void tesselateNeigh<0, 1, 0>(Block const *p, Block const *q, int3 const &position, VertexArray &vertices, NormalArray &normals, unsigned &end) {
	p += CHUNK_SIZE * (CHUNK_SIZE - 1);
	for (unsigned z = 0; z < CHUNK_SIZE; ++z) {
		for (unsigned x = 0; x < CHUNK_SIZE; ++x) {
			tess<0, 1, 0>(*p, *q, x, CHUNK_SIZE - 1, z, position, vertices, normals, end);
			++p;
			++q;
		}
		p += CHUNK_SIZE * (CHUNK_SIZE - 1);
		q += CHUNK_SIZE * (CHUNK_SIZE - 1);
	}
}

template<>
void tesselateNeigh<0, 0, -1>(Block const *p, Block const *q, int3 const &position, VertexArray &vertices, NormalArray &normals, unsigned &end) {
	q += CHUNK_SIZE * CHUNK_SIZE * (CHUNK_SIZE - 1);
	for (unsigned y = 0; y < CHUNK_SIZE; ++y) {
		for (unsigned x = 0; x < CHUNK_SIZE; ++x) {
			tess<0, 0, -1>(*p, *q, x, y, (unsigned)0, position, vertices, normals, end);
			++p;
			++q;
		}
	}
}

template<>
void tesselateNeigh<0, 0, 1>(Block const *p, Block const *q, int3 const &position, VertexArray &vertices, NormalArray &normals, unsigned &end) {
	p += CHUNK_SIZE * CHUNK_SIZE * (CHUNK_SIZE - 1);
	for (unsigned y = 0; y < CHUNK_SIZE; ++y) {
		for (unsigned x = 0; x < CHUNK_SIZE; ++x) {
			tess<0, 0, 1>(*p, *q, x, y, CHUNK_SIZE - 1, position, vertices, normals, end);
			++p;
			++q;
		}
	}
}

template<int dx, int dy, int dz>
void tesselateFace(ChunkDataPtr data, ChunkDataPtr neighData, int3 const &position, ChunkGeometryPtr geometry) {
	static int const neighOffset = dx + (int)CHUNK_SIZE * dy + (int)CHUNK_SIZE * (int)CHUNK_SIZE * dz;

	VertexArray &vertices = geometry->getVertexData();
	NormalArray &normals = geometry->getNormalData();

	unsigned const begin = vertices.size();

	unsigned end = begin;
	Block const *rawData = data->raw();
	Block const *rawNeighData = neighData->raw();
	Block const *p = rawData + CHUNK_SIZE * CHUNK_SIZE + CHUNK_SIZE + 1;
	for (unsigned z = 1; z < CHUNK_SIZE - 1; ++z) {
		for (unsigned y = 1; y < CHUNK_SIZE - 1; ++y) {
			for (unsigned x = 1; x < CHUNK_SIZE - 1; ++x) {
				tess<dx, dy, dz>(*p, p[neighOffset], x, y, z, position, vertices, normals, end);
				++p;
			}
			p += 2;
		}
		p += 2 * CHUNK_SIZE;
	}

	tesselateNeigh<dx, dy, dz>(rawData, rawNeighData, position, vertices, normals, end);

	stats.quadsGenerated.increment(vertices.size() / 4);

	geometry->setRange(FaceIndex<dx, dy, dz>::value, Range(begin, end));
}

void tesselate(ChunkDataPtr data, NeighbourChunkData const &neighbourData, int3 const &position, ChunkGeometryPtr geometry) {
	SafeTimer::Timed t = stats.chunkTesselationTime.timed();

	tesselateFace<-1,  0,  0>(data, neighbourData.xn, position, geometry);
	tesselateFace< 1,  0,  0>(data, neighbourData.xp, position, geometry);
	tesselateFace< 0, -1,  0>(data, neighbourData.yn, position, geometry);
	tesselateFace< 0,  1,  0>(data, neighbourData.yp, position, geometry);
	tesselateFace< 0,  0, -1>(data, neighbourData.zn, position, geometry);
	tesselateFace< 0,  0,  1>(data, neighbourData.zp, position, geometry);

	stats.chunksTesselated.increment();
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
	buffers->setRanges(geometry.getRanges());
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
}

Chunk::Chunk(int3 const &index)
:
	index(index),
	position(CHUNK_SIZE * index.x, CHUNK_SIZE * index.y, CHUNK_SIZE * index.z),
	state(NEW)
{
}

void Chunk::setGenerating() {
	state = GENERATING;
}

void Chunk::setData(ChunkDataPtr data) {
	this->data = data;
	state = GENERATED;
}

void Chunk::setTesselating() {
	state = TESSELATING;
}

void Chunk::setGeometry(ChunkGeometryPtr geometry) {
	this->geometry = geometry;
	state = TESSELATED;
}

void Chunk::render() {
	if (state < TESSELATED) {
		return;
	}
	if (state == TESSELATED) {
		upload();
	}
	if (state == UPLOADED) {
		if (buffers) {
			::render(*buffers);
			stats.chunksRendered.increment();
		} else {
			stats.chunksEmpty.increment();
		}
	}
}

void Chunk::upload() { 
	if (geometry) {
		if (!geometry->isEmpty()) {
			buffers.reset(new ChunkBuffers());
			::upload(*geometry, buffers.get());
		}
		geometry.reset();
	}
	state = UPLOADED;
}
