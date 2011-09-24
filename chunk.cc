#include "chunk.h"

#include "terragen.h"

const unsigned CHUNK_SIZE = 32;

const unsigned BLOCKS_PER_CHUNK = CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE;

ChunkData::CoordsIterator::CoordsIterator(int3 const &coords)
:
	coords(coords)
{
}

ChunkData::CoordsIterator &ChunkData::CoordsIterator::operator++() {
	++coords.x;
	coords.y += coords.x / CHUNK_SIZE;
	coords.x %= CHUNK_SIZE;
	coords.z += coords.y / CHUNK_SIZE;
	coords.y %= CHUNK_SIZE;
	return *this;
}

ChunkData::CoordsIterator ChunkData::CoordsIterator::operator++(int) {
	CoordsIterator old = *this;
	++*this;
	return old;
}

bool ChunkData::CoordsIterator::operator==(ChunkData::CoordsIterator const &other) const {
	return coords == other.coords;
}

bool ChunkData::CoordsIterator::operator!=(ChunkData::CoordsIterator const &other) const {
	return !(*this == other);
}

int3 const &ChunkData::CoordsIterator::operator*() const {
	return coords;
}

ChunkData::ChunkData() {
	blocks.reset(new Block[BLOCKS_PER_CHUNK]);
	for (iterator i = begin(); i != end(); ++i) {
		*i = AIR_BLOCK;
	}
}

Block &ChunkData::at(int3 pos) {
	return blocks[CHUNK_SIZE * CHUNK_SIZE * pos.z + CHUNK_SIZE * pos.y + pos.x];
}

Block const &ChunkData::at(int3 pos) const {
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

ChunkData::coords_iterator ChunkData::beginCoords() const {
	return CoordsIterator(int3(0, 0, 0));
}

ChunkData::coords_iterator ChunkData::endCoords() const {
	return CoordsIterator(int3(0, 0, CHUNK_SIZE));
}

Chunk::Chunk(int3 const &pos, TerrainGenerator &terrainGenerator)
:
	pos(pos)
{
	terrainGenerator.generateChunk(data, pos);

	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
	float const d = CHUNK_SIZE;
	float const x = d * pos.x, y = d * pos.y, z = d * pos.z;
	float const X = x + d, Y = y + d, Z = z + d;
	float vertices[] = {
		x, y, z, x, y, Z, x, Y, Z, x, Y, z,
		X, y, z, X, Y, z, X, Y, Z, X, y, Z,
		x, y, z, X, y, z, X, y, Z, x, y, Z,
		x, Y, z, x, Y, Z, X, Y, Z, X, Y, z,
		x, y, z, x, Y, z, X, Y, z, X, y, z,
		x, y, Z, X, y, Z, X, Y, Z, x, Y, Z,
	};
	glBufferData(GL_ARRAY_BUFFER, 6 * 4 * 3 * sizeof(float), vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, normalBuffer);
	float normals[] = {
		-1,  0,  0, -1,  0,  0, -1,  0,  0, -1,  0,  0,
		 1,  0,  0,  1,  0,  0,  1,  0,  0,  1,  0,  0,
		 0, -1,  0,  0, -1,  0,  0, -1,  0,  0, -1,  0,
		 0,  1,  0,  0,  1,  0,  0,  1,  0,  0,  1,  0,
		 0,  0, -1,  0,  0, -1,  0,  0, -1,  0,  0, -1,
		 0,  0,  1,  0,  0,  1,  0,  0,  1,  0,  0,  1,
	};
	glBufferData(GL_ARRAY_BUFFER, 6 * 4 * 3 * sizeof(float), normals, GL_STATIC_DRAW);
}

void Chunk::render() const {
	glEnableClientState(GL_VERTEX_ARRAY);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
	glVertexPointer(3, GL_FLOAT, 0, 0);

	glEnableClientState(GL_NORMAL_ARRAY);
	glBindBuffer(GL_ARRAY_BUFFER, normalBuffer);
	glNormalPointer(GL_FLOAT, 0, 0);

	glDrawArrays(GL_QUADS, 0, 6 * 4);
}
