#include "chunkdata.h"

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
