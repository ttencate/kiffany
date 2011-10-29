#include "lighting.h"

void computeLighting(int3 index, ChunkMap const &chunkMap, ChunkGeometryPtr geometry) {
	NormalArray &normals = geometry->getNormalData();
	for (unsigned i = 0; i < normals.size(); ++i) {
		normals[i] /= 2;
	}
}
