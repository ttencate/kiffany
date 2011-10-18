#include "octree.h"

#include "chunkdata.h"

Octree::Octree()
:
	nodes()
{
}

Block determineType(unsigned size, Block const *base) {
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
			nodes.push_back(OctreeNode(block));
			if (block == INVALID_BLOCK) {
				unsigned const s = size / 2;
				unsigned const sx = s;
				unsigned const sy = CHUNK_SIZE * s;
				unsigned const sz = CHUNK_SIZE * CHUNK_SIZE * s;
				// Don't store a pointer/reference to the new node,
				// since the vector might reallocate in the meantime.
				nodes[i].children[0] = subdivideOctree(s, base               , nodes);
				nodes[i].children[1] = subdivideOctree(s, base +           sx, nodes);
				nodes[i].children[2] = subdivideOctree(s, base +      sy     , nodes);
				nodes[i].children[3] = subdivideOctree(s, base +      sy + sx, nodes);
				nodes[i].children[4] = subdivideOctree(s, base + sz          , nodes);
				nodes[i].children[5] = subdivideOctree(s, base + sz +      sx, nodes);
				nodes[i].children[6] = subdivideOctree(s, base + sz + sy     , nodes);
				nodes[i].children[7] = subdivideOctree(s, base + sz + sy + sx, nodes);
			}
			return i;
		}
	}
}

void buildOctree(RawChunkData const &rawChunkData, Octree &octree) {
	octree = Octree();
	Block const *raw = rawChunkData.raw();
	subdivideOctree(CHUNK_SIZE, raw, octree.getNodes());
}
