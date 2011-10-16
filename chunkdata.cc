#include "chunkdata.h"

#include "stats.h"

#include <limits>

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

RleCompressor::RleCompressor(RleChunkDataPtr rleChunkData)
:
	rleChunkData(rleChunkData)
{
	rleChunkData->getRuns().clear();
	currentRun.block = AIR_BLOCK;
	currentRun.length = 0;
}

RleCompressor::~RleCompressor() {
	pushRun();
}

void RleCompressor::put(Block block) {
	if (currentRun.block != block || currentRun.length >= std::numeric_limits<RleChunkData::Length>::max()) {
		pushRun();
		currentRun.block = block;
		currentRun.length = 0;
	}
	++currentRun.length;
}

void RleCompressor::pushRun() {
	if (currentRun.length > 0) {
		rleChunkData->getRuns().push_back(currentRun);
	}
}

RleDecompressor::RleDecompressor(RleChunkDataPtr rleChunkData)
:
	rleChunkData(rleChunkData),
	nextRunIndex(0)
{
	currentRun.block = AIR_BLOCK;
	currentRun.length = 0;
}

Block RleDecompressor::get() {
	while (currentRun.length == 0 && nextRunIndex < rleChunkData->getRuns().size()) {
		currentRun = rleChunkData->getRuns()[nextRunIndex];
		++nextRunIndex;
	}
	if (currentRun.length > 0) {
		--currentRun.length;
	}
	return currentRun.block;
}

void compress(ChunkDataPtr chunkData, RleChunkDataPtr rleChunkData) {
	Block const *data = chunkData->raw();
	Block const *const end = data + BLOCKS_PER_CHUNK;

	RleCompressor compressor(rleChunkData);

	for (; data < end; ++data) {
		compressor.put(*data);
	}
}

void decompress(RleChunkDataPtr rleChunkData, ChunkDataPtr chunkData) {
	RleDecompressor decompressor(rleChunkData);

	Block *data = chunkData->raw();
	Block *const end = data + BLOCKS_PER_CHUNK;

	for (; data < end; ++data) {
		*data = decompressor.get();
	}
}
