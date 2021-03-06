#include "octree.h"

#include "chunkdata.h"
#include "stats.h"

void Octree::getBlock(int3 position, Block *block, int3 *base, unsigned *size) const {
	*base = int3(0, 0, 0);
	*size = CHUNK_SIZE;
	if (nodes.size() == 0) {
		*block = AIR_BLOCK;
		return;
	}
	unsigned nodeIndex = 0;
	for (int mask = CHUNK_SIZE >> 1;; mask >>= 1) {
		OctreeNode const &node = nodes[nodeIndex];
		// TODO: If !mask, we must find a valid block; assert this.
		if (node.block != INVALID_BLOCK) {
			*block = node.block;
			break;
		}
		unsigned childIndexInParent =
			((position.x & mask) ? 1 : 0) |
			((position.y & mask) ? 2 : 0) |
			((position.z & mask) ? 4 : 0);
		nodeIndex = node.children[childIndexInParent];
		*size >>= 1;
		*base += int3(
				(position.x & mask) ? *size : 0,
				(position.y & mask) ? *size : 0,
				(position.z & mask) ? *size : 0);
		if (!nodeIndex) {
			*block = AIR_BLOCK;
			break;
		}
	}
}

inline Block determineType(unsigned size, Block const *base) {
	Block const block = *base;
	for (unsigned z = 0; z < size; ++z) {
		for (unsigned y = 0; y < size; ++y) {
			for (unsigned x = 0; x < size; ++x) {
				Block const b = *base;
				if (b != block) {
					return INVALID_BLOCK;
				}
				++base;
			}
			base += CHUNK_SIZE - size;
		}
		base += CHUNK_SIZE * (CHUNK_SIZE - size);
	}
	return block;
}

unsigned subdivideOctree(unsigned size, Block const *base, OctreeNodes &nodes) {
	if (size == 1) {
		if (*base == AIR_BLOCK) {
			return 0; // AIR_BLOCK is implicit
		} else {
			nodes.push_back(OctreeNode(*base));
			return nodes.size() - 1;
		}
	} else {
		Block const block = determineType(size, base);
		if (block == AIR_BLOCK) {
			return 0;
		} else {
			unsigned const i = nodes.size();
			// Don't operate on the new node directly,
			// since the vector might reallocate in the meantime.
			nodes.push_back(OctreeNode());
			OctreeNode node(block);
			if (block == INVALID_BLOCK) {
				unsigned const s = size / 2;
				unsigned const sx = s;
				unsigned const sy = CHUNK_SIZE * s;
				unsigned const sz = CHUNK_SIZE * CHUNK_SIZE * s;
				node.children[0] = subdivideOctree(s, base               , nodes);
				node.children[1] = subdivideOctree(s, base +           sx, nodes);
				node.children[2] = subdivideOctree(s, base +      sy     , nodes);
				node.children[3] = subdivideOctree(s, base +      sy + sx, nodes);
				node.children[4] = subdivideOctree(s, base + sz          , nodes);
				node.children[5] = subdivideOctree(s, base + sz +      sx, nodes);
				node.children[6] = subdivideOctree(s, base + sz + sy     , nodes);
				node.children[7] = subdivideOctree(s, base + sz + sy + sx, nodes);
			}
			nodes[i] = node;
			return i;
		}
	}
}

void buildOctree(RawChunkData const &rawChunkData, Octree &octree) {
	TimerStat::Timed timed = stats.octreeBuildTime.timed();
	octree = Octree();
	Block const *raw = rawChunkData.raw();
	subdivideOctree(CHUNK_SIZE, raw, octree.getNodes());
	stats.octreesBuilt.increment();
	stats.octreeNodes.increment(octree.getNodes().size());
}

void unpackOctreeNodes(unsigned size, unsigned index, OctreeNodes const &nodes, Block *base) {
	Block block = AIR_BLOCK;
	if ((index != 0 || size == CHUNK_SIZE) && index < nodes.size()) {
		block = nodes[index].block;
	}
	if (block == INVALID_BLOCK) {
		OctreeNode const &node = nodes[index];
		unsigned const s = size / 2;
		unsigned const sx = s;
		unsigned const sy = CHUNK_SIZE * s;
		unsigned const sz = CHUNK_SIZE * CHUNK_SIZE * s;
		unpackOctreeNodes(s, node.children[0], nodes, base               );
		unpackOctreeNodes(s, node.children[1], nodes, base +           sx);
		unpackOctreeNodes(s, node.children[2], nodes, base +      sy     );
		unpackOctreeNodes(s, node.children[3], nodes, base +      sy + sx);
		unpackOctreeNodes(s, node.children[4], nodes, base + sz          );
		unpackOctreeNodes(s, node.children[5], nodes, base + sz +      sx);
		unpackOctreeNodes(s, node.children[6], nodes, base + sz + sy     );
		unpackOctreeNodes(s, node.children[7], nodes, base + sz + sy + sx);
	} else {
		for (unsigned z = 0; z < size; ++z) {
			for (unsigned y = 0; y < size; ++y) {
				for (unsigned x = 0; x < size; ++x) {
					*base = block;
					++base;
				}
				base += CHUNK_SIZE - size;
			}
			base += CHUNK_SIZE * (CHUNK_SIZE - size);
		}
	}
}

void unpackOctree(Octree const &octree, RawChunkData &rawChunkData) {
	TimerStat::Timed timed = stats.octreeUnpackTime.timed();
	unpackOctreeNodes(CHUNK_SIZE, 0, octree.getNodes(), rawChunkData.raw());
	stats.octreesUnpacked.increment();
}
