#include "chunkdata.h"

#include "stats.h"

ChunkData::ChunkData() {
	blocks.reset(new Block[BLOCKS_PER_CHUNK]);
	for (iterator i = begin(); i != end(); ++i) {
		*i = AIR_BLOCK;
	}
}

Block &ChunkData::operator[](int3 pos) {
	return blocks[CHUNK_SIZE * CHUNK_SIZE * pos.z + CHUNK_SIZE * pos.y + pos.x];
}

Block const &ChunkData::operator[](int3 pos) const {
	return blocks[CHUNK_SIZE * CHUNK_SIZE * pos.z + CHUNK_SIZE * pos.y + pos.x];
}

ChunkData::iterator ChunkData::begin() {
	return &blocks[0];
}

ChunkData::iterator ChunkData::end() {
	return &blocks[BLOCKS_PER_CHUNK];
}

ChunkData::const_iterator ChunkData::begin() const {
	return &blocks[0];
}

ChunkData::const_iterator ChunkData::end() const {
	return &blocks[BLOCKS_PER_CHUNK];
}

CoordsBlock ChunkData::getCoordsBlock() const {
	return CoordsBlock(int3(CHUNK_SIZE));
}

Block const *ChunkData::raw() const {
	return &blocks[0];
}

Block *ChunkData::raw() {
	return &blocks[0];
}

bool ChunkGeometry::isEmpty() const {
	return vertexData.size() == 0;
}

void tesselate(ChunkData const &data, int3 const &position, ChunkGeometry *geometry) {
	char const N = 0x7F;

	Timed t = stats.chunkTesselationTime.timed();

	std::vector<short> &vertices = geometry->getVertexData();
	std::vector<char> &normals = geometry->getNormalData();
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
					int3 m = (int3)blockMin(position + pos); // TODO make relative, use matrix
					int3 M = (int3)blockMax(position + pos);
					if (x == 0 || needsDrawing(block, p[-1])) {
						short v[] = { m.x, m.y, m.z, m.x, m.y, M.z, m.x, M.y, M.z, m.x, M.y, m.z };
						char n[] = { -N, 0, 0, -N, 0, 0, -N, 0, 0, -N, 0, 0 };
						vertices.resize(s + 12);
						normals.resize(s + 12);
						memcpy(&vertices[s], v, 12 * sizeof(short));
						memcpy(&normals[s], n, 12 * sizeof(char));
						s += 12;
					}
					if (x == CHUNK_SIZE - 1 || needsDrawing(block, p[1])) {
						short v[] = { M.x, m.y, m.z, M.x, M.y, m.z, M.x, M.y, M.z, M.x, m.y, M.z };
						char n[] = { N, 0, 0, N, 0, 0, N, 0, 0, N, 0, 0 };
						vertices.resize(s + 12);
						normals.resize(s + 12);
						memcpy(&vertices[s], v, 12 * sizeof(short));
						memcpy(&normals[s], n, 12 * sizeof(char));
						s += 12;
					}
					if (y == 0 || needsDrawing(block, p[-(int)CHUNK_SIZE])) {
						short v[] = { m.x, m.y, m.z, M.x, m.y, m.z, M.x, m.y, M.z, m.x, m.y, M.z };
						char n[] = { 0, -N, 0, 0, -N, 0, 0, -N, 0, 0, -N, 0 };
						vertices.resize(s + 12);
						normals.resize(s + 12);
						memcpy(&vertices[s], v, 12 * sizeof(short));
						memcpy(&normals[s], n, 12 * sizeof(char));
						s += 12;
					}
					if (y == CHUNK_SIZE - 1 || needsDrawing(block, p[CHUNK_SIZE])) {
						short v[] = { m.x, M.y, m.z, m.x, M.y, M.z, M.x, M.y, M.z, M.x, M.y, m.z };
						char n[] = { 0, N, 0, 0, N, 0, 0, N, 0, 0, N, 0 };
						vertices.resize(s + 12);
						normals.resize(s + 12);
						memcpy(&vertices[s], v, 12 * sizeof(short));
						memcpy(&normals[s], n, 12 * sizeof(char));
						s += 12;
					}
					if (z == 0 || needsDrawing(block, p[-(int)CHUNK_SIZE * (int)CHUNK_SIZE])) {
						short v[] = { m.x, m.y, m.z, m.x, M.y, m.z, M.x, M.y, m.z, M.x, m.y, m.z };
						char n[] = { 0, 0, -N, 0, 0, -N, 0, 0, -N, 0, 0, -N };
						vertices.resize(s + 12);
						normals.resize(s + 12);
						memcpy(&vertices[s], v, 12 * sizeof(short));
						memcpy(&normals[s], n, 12 * sizeof(char));
						s += 12;
					}
					if (z == CHUNK_SIZE - 1 || needsDrawing(block, p[CHUNK_SIZE * CHUNK_SIZE])) {
						short v[] = { m.x, m.y, M.z, M.x, m.y, M.z, M.x, M.y, M.z, m.x, M.y, M.z };
						char n[] = { 0, 0, N, 0, 0, N, 0, 0, N, 0, 0, N };
						vertices.resize(s + 12);
						normals.resize(s + 12);
						memcpy(&vertices[s], v, 12 * sizeof(short));
						memcpy(&normals[s], n, 12 * sizeof(char));
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
