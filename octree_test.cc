#include <boost/test/unit_test.hpp>

#include "chunkdata.h"
#include "octree.h"

BOOST_AUTO_TEST_SUITE(OctreeTest)

void testConstruction(RawChunkData const &data) {
	Octree octree;
	buildOctree(data, octree);
	for (unsigned z = 0; z < CHUNK_SIZE; ++z) {
		for (unsigned y = 0; y < CHUNK_SIZE; ++y) {
			for (unsigned x = 0; x < CHUNK_SIZE; ++x) {
				Block expected = data.raw()[x + CHUNK_SIZE * y + CHUNK_SIZE * CHUNK_SIZE * z];
				Block actual;
				int3 base;
				unsigned size;
				octree.getBlock(int3(x, y, z), &actual, &base, &size);
				BOOST_REQUIRE_EQUAL(expected, actual);
			}
		}
	}
}

void empty(RawChunkData &data) {
}

void full(RawChunkData &data) {
	Block *raw = data.raw();
	for (unsigned i = 0; i < BLOCKS_PER_CHUNK; ++i) {
		raw[i] = STONE_BLOCK;
	}
}

void halfFull(RawChunkData &data) {
	Block *raw = data.raw();
	for (unsigned i = 0; i < BLOCKS_PER_CHUNK; ++i) {
		raw[i] = (i < BLOCKS_PER_CHUNK / 2) ? STONE_BLOCK : AIR_BLOCK;
	}
}

void singleBlock(RawChunkData &data, unsigned x, unsigned y, unsigned z) {
	data.raw()[x + CHUNK_SIZE * y + CHUNK_SIZE * CHUNK_SIZE * z] = STONE_BLOCK;
}

void random(RawChunkData &data) {
	Block *raw = data.raw();
	srand(0);
	for (unsigned i = 0; i < BLOCKS_PER_CHUNK; ++i) {
		raw[i] = (rand() % 2 == 1) ? STONE_BLOCK : AIR_BLOCK;
	}
}

BOOST_AUTO_TEST_CASE(TestConstructEmpty) {
	RawChunkData data;
	empty(data);
	Octree octree;
	buildOctree(data, octree);
	BOOST_REQUIRE_EQUAL(0, octree.getNodes().size());
}

BOOST_AUTO_TEST_CASE(TestConstructFull) {
	RawChunkData data;
	full(data);
	Octree octree;
	buildOctree(data, octree);
	BOOST_REQUIRE_EQUAL(1, octree.getNodes().size());
	BOOST_REQUIRE_EQUAL((Block)STONE_BLOCK, octree.getNodes()[0].block);
}

BOOST_AUTO_TEST_CASE(TestConstructHalfFull) {
	RawChunkData data;
	halfFull(data);
	Octree octree;
	buildOctree(data, octree);
	BOOST_REQUIRE_EQUAL(5, octree.getNodes().size());
	BOOST_REQUIRE_EQUAL((Block)INVALID_BLOCK, octree.getNodes()[0].block);
	unsigned children[8] = { 1, 2, 3, 4, 0, 0, 0, 0 };
	BOOST_REQUIRE_EQUAL_COLLECTIONS(children, children + 8,
			octree.getNodes()[0].children, octree.getNodes()[0].children + 8);
}

BOOST_AUTO_TEST_CASE(TestConstructCornerBlock) {
	RawChunkData data;
	singleBlock(data, 0, 0, 0);
	Octree octree;
	buildOctree(data, octree);
	BOOST_REQUIRE_EQUAL(CHUNK_POWER + 1, octree.getNodes().size());
	for (unsigned i = 0; i < CHUNK_POWER; ++i) {
		BOOST_REQUIRE_EQUAL((Block)INVALID_BLOCK, octree.getNodes()[i].block);
	}
	BOOST_REQUIRE_EQUAL((Block)STONE_BLOCK, octree.getNodes()[CHUNK_POWER].block);
}

BOOST_AUTO_TEST_CASE(TestConstructCenterBlock) {
	RawChunkData data;
	singleBlock(data, CHUNK_SIZE / 2, CHUNK_SIZE / 2, CHUNK_SIZE / 2);
	Octree octree;
	buildOctree(data, octree);
	BOOST_REQUIRE_EQUAL(CHUNK_POWER + 1, octree.getNodes().size());
	for (unsigned i = 0; i < CHUNK_POWER; ++i) {
		BOOST_REQUIRE_EQUAL((Block)INVALID_BLOCK, octree.getNodes()[i].block);
	}
	BOOST_REQUIRE_EQUAL((Block)STONE_BLOCK, octree.getNodes()[CHUNK_POWER].block);
}

BOOST_AUTO_TEST_CASE(TestDeconstructEmpty) {
	RawChunkData data;
	empty(data);
	testConstruction(data);
}

BOOST_AUTO_TEST_CASE(TestDeconstructFull) {
	RawChunkData data;
	full(data);
	testConstruction(data);
}

BOOST_AUTO_TEST_CASE(TestDeconstructHalfFull) {
	RawChunkData data;
	halfFull(data);
	testConstruction(data);
}

BOOST_AUTO_TEST_CASE(TestDeconstructCornerBlock) {
	RawChunkData data;
	singleBlock(data, 0, 0, 0);
	testConstruction(data);
}

BOOST_AUTO_TEST_CASE(TestDeconstructCenterBlock) {
	RawChunkData data;
	singleBlock(data, CHUNK_SIZE / 2, CHUNK_SIZE / 2, CHUNK_SIZE / 2);
	testConstruction(data);
}

BOOST_AUTO_TEST_CASE(TestDeconstructRandom) {
	RawChunkData data;
	random(data);
	testConstruction(data);
}

BOOST_AUTO_TEST_SUITE_END()
