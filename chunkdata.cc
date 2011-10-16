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

RleCompressor::RleCompressor(ChunkData &chunkData)
:
	chunkData(chunkData)
{
	chunkData.getRuns().clear();
	currentRun.block = AIR_BLOCK;
	currentRun.length = 0;
}

RleCompressor::~RleCompressor() {
	pushRun();
}

void RleCompressor::put(Block block) {
	if (currentRun.block != block || currentRun.length >= std::numeric_limits<ChunkData::Length>::max()) {
		pushRun();
		currentRun.block = block;
		currentRun.length = 0;
	}
	++currentRun.length;
}

void RleCompressor::pushRun() {
	if (currentRun.length > 0) {
		chunkData.getRuns().push_back(currentRun);
	}
}

RleDecompressor::RleDecompressor(ChunkData const &chunkData)
:
	chunkData(chunkData),
	nextRunIndex(0)
{
	currentRun.block = AIR_BLOCK;
	currentRun.length = 0;
}

Block RleDecompressor::get() {
	while (currentRun.length == 0 && nextRunIndex < chunkData.getRuns().size()) {
		currentRun = chunkData.getRuns()[nextRunIndex];
		++nextRunIndex;
	}
	if (currentRun.length > 0) {
		--currentRun.length;
	}
	return currentRun.block;
}

void compress(RawChunkData const &rawChunkData, ChunkData &chunkData) {
	Block const *data = rawChunkData.raw();
	Block const *const end = data + BLOCKS_PER_CHUNK;

	RleCompressor compressor(chunkData);

	for (; data < end; ++data) {
		compressor.put(*data);
	}
}

void decompress(ChunkData const &chunkData, RawChunkData &rawChunkData) {
	RleDecompressor decompressor(chunkData);

	Block *data = rawChunkData.raw();
	Block *const end = data + BLOCKS_PER_CHUNK;

	for (; data < end; ++data) {
		*data = decompressor.get();
	}
}
