#ifndef CHUNKDATA_H
#define CHUNKDATA_H

#include "block.h"
#include "coords.h"
#include "maths.h"

#include <boost/noncopyable.hpp>
#include <boost/scoped_array.hpp>
#include <boost/shared_ptr.hpp>

#include <vector>

class ChunkData
:
	boost::noncopyable
{

	boost::scoped_array<Block> blocks;

	public:

		typedef Block* iterator;
		typedef Block const* const_iterator;

		ChunkData();

		Block &operator[](int3 pos);
		Block const &operator[](int3 pos) const;

		iterator begin();
		iterator end();
		const_iterator begin() const;
		const_iterator end() const;

		CoordsBlock getCoordsBlock() const;

		Block const *raw() const;
		Block *raw();

};

typedef boost::shared_ptr<ChunkData> ChunkDataPtr;

class RleChunkData
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

};

typedef boost::shared_ptr<RleChunkData> RleChunkDataPtr;

class RleCompressor {

	RleChunkDataPtr const rleChunkData;
	RleChunkData::Run currentRun;

	public:

		RleCompressor(RleChunkDataPtr rleChunkData);
		~RleCompressor();

		void put(Block block);

	private:

		void pushRun();

};

class RleDecompressor {

	RleChunkDataPtr const rleChunkData;
	RleChunkData::Run currentRun;
	unsigned nextRunIndex;

	public:

		RleDecompressor(RleChunkDataPtr rleChunkData);

		Block get();

};

void compress(ChunkDataPtr chunkData, RleChunkDataPtr rleChunkData);
void decompress(RleChunkDataPtr rleChunkData, ChunkDataPtr chunkData);

#endif
