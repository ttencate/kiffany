#ifndef CHUNKDATA_H
#define CHUNKDATA_H

#include "block.h"
#include "coords.h"
#include "maths.h"

#include <boost/noncopyable.hpp>
#include <boost/scoped_array.hpp>
#include <boost/shared_ptr.hpp>

#include <vector>

class RawChunkData
:
	boost::noncopyable
{

	boost::scoped_array<Block> blocks;

	public:

		typedef Block* iterator;
		typedef Block const* const_iterator;

		RawChunkData();

		Block &operator[](int3 pos);
		Block const &operator[](int3 pos) const;

		iterator begin();
		iterator end();
		const_iterator begin() const;
		const_iterator end() const;

		Block const *raw() const;
		Block *raw();

};

typedef boost::shared_ptr<RawChunkData> RawChunkDataPtr;

class ChunkData
:
	boost::noncopyable
{
	public:

		typedef unsigned Length;

		struct Run {
			Block block;
			Length length;
		};

		typedef std::vector<Run> Runs;

	private:

		Runs runs;

	public:

		Runs &getRuns() { return runs; }
		Runs const &getRuns() const { return runs; }

		bool isEmpty() const;

};

typedef boost::shared_ptr<ChunkData> ChunkDataPtr;

class RleCompressor {

	ChunkData &chunkData;
	ChunkData::Run currentRun;

	public:

		RleCompressor(ChunkData &chunkData);
		~RleCompressor();

		void put(Block block);

	private:

		void pushRun();

};

class RleDecompressor {

	ChunkData const &chunkData;
	ChunkData::Run currentRun;
	unsigned nextRunIndex;

	public:

		RleDecompressor(ChunkData const &chunkData);

		Block get();

};

void compress(RawChunkData const &rawChunkData, ChunkData &chunkData);
void decompress(ChunkData const &chunkData, RawChunkData &rawChunkData);

#endif
