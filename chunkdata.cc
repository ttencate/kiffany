#include "chunkdata.h"

#include "stats.h"

#include <limits>

RawChunkData::RawChunkData() {
	blocks.reset(new Block[BLOCKS_PER_CHUNK]);
	for (iterator i = begin(); i != end(); ++i) {
		*i = AIR_BLOCK;
	}
}

Block &RawChunkData::operator[](int3 pos) {
	return blocks[CHUNK_SIZE * CHUNK_SIZE * pos.z + CHUNK_SIZE * pos.y + pos.x];
}

Block const &RawChunkData::operator[](int3 pos) const {
	return blocks[CHUNK_SIZE * CHUNK_SIZE * pos.z + CHUNK_SIZE * pos.y + pos.x];
}

RawChunkData::iterator RawChunkData::begin() {
	return &blocks[0];
}

RawChunkData::iterator RawChunkData::end() {
	return &blocks[BLOCKS_PER_CHUNK];
}

RawChunkData::const_iterator RawChunkData::begin() const {
	return &blocks[0];
}

RawChunkData::const_iterator RawChunkData::end() const {
	return &blocks[BLOCKS_PER_CHUNK];
}

Block const *RawChunkData::raw() const {
	return &blocks[0];
}

Block *RawChunkData::raw() {
	return &blocks[0];
}
