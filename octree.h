#ifndef OCTREE_H
#define OCTREE_H

#include "block.h"

#include <boost/shared_ptr.hpp>

#include <cstring>
#include <vector>

struct OctreeNode {

	Block block;
	unsigned children[8];

	explicit OctreeNode(Block block = INVALID_BLOCK)
	:
		block(block)
	{
		memset(children, 0, sizeof(children));
	}

};

typedef std::vector<OctreeNode> OctreeNodes;

class Octree {

	/* A node has block == INVALID_BLOCK iff it has children.
	 * If a child i does not exist, the corresponding entry children[i] is 0,
	 * and the child is implicitly filled with AIR_BLOCK.
	 * Node 0 is the root. If it is not there, the entire chunk is AIR_BLOCK.
	 */
	OctreeNodes nodes;

	public:

		OctreeNodes &getNodes() { return nodes; }
		OctreeNodes const &getNodes() const { return nodes; }

};

typedef boost::shared_ptr<Octree> OctreePtr;

class RawChunkData;

void buildOctree(RawChunkData const &rawChunkData, Octree &octree);

#endif
