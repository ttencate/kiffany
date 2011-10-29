#include "lighting.h"
#include "raycaster.h"

void computeLighting(int3 index, ChunkMap const &chunkMap, ChunkGeometryPtr geometry) {
	float const cutoff = CHUNK_SIZE;
	Raycaster raycast(chunkMap, cutoff, STONE_BLOCK, BLOCK_MASK);

	VertexArray::value_type const *vertices = &geometry->getVertexData()[0];
	NormalArray::value_type *normals = &geometry->getNormalData()[0];
	unsigned const numQuads = geometry->getNumQuads();
	for (unsigned i = 0; i < numQuads; ++i) {
		vec3 const a(vertices[0], vertices[1], vertices[2]);
		vec3 const b(vertices[3], vertices[4], vertices[5]);
		vec3 const c(vertices[6], vertices[7], vertices[8]);
		vec3 const baseNormal = cross(c - b, a - b); // No need to normalize, edges are 1.

		vec3 const center = 0.5f * (a + c);

		RaycastResult result = raycast(index, center, baseNormal);
		vec3 normal = baseNormal;
		if (result.status == RaycastResult::HIT) {
			normal *= result.length / cutoff;
		}

		normals[0] = (int)(0x7F * normal.x);
		normals[1] = (int)(0x7F * normal.y);
		normals[2] = (int)(0x7F * normal.z);
		normals[3] = normals[0];
		normals[4] = normals[1];
		normals[5] = normals[2];
		normals[6] = normals[0];
		normals[7] = normals[1];
		normals[8] = normals[2];
		normals[9] = normals[0];
		normals[10] = normals[1];
		normals[11] = normals[2];

		vertices += 12;
		normals += 12;
	}
}
