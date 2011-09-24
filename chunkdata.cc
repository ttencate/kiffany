#include "chunkdata.h"

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

ChunkData::coords_iterator ChunkData::beginCoords() const {
	return CoordsIterator(int3(0, 0, 0));
}

ChunkData::coords_iterator ChunkData::endCoords() const {
	return CoordsIterator(int3(0, 0, CHUNK_SIZE));
}

vec3 blockMin(int3 const &pos) {
	return vec3(pos);
}

vec3 blockMax(int3 const &pos) {
	return vec3(pos) + vec3(1.0f);
}

vec3 blockCenter(int3 const &pos) {
	return vec3(pos.x + 0.5f, pos.y + 0.5f, pos.z + 0.5f);
}
