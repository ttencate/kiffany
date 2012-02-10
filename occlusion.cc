#include "occlusion.h"
#include "raycaster.h"
#include "stats.h"

void computeLighting(int3 index, ChunkMap const &chunkMap, ChunkGeometryPtr geometry) {
	TimerStat::Timed t(stats.chunkLightingTime.timed());

	float const cutoff = CHUNK_SIZE;
	Raycaster raycast(chunkMap, cutoff, STONE_BLOCK, BLOCK_MASK);

	VertexArray::value_type const *vertices = &geometry->getVertexData()[0];
	NormalArray::value_type *normals = &geometry->getNormalData()[0];
	unsigned const numQuads = geometry->getNumQuads();
	for (unsigned i = 0; i < numQuads; ++i) {
		vec3 const a(vertices[0], vertices[1], vertices[2]);
		vec3 const b(vertices[3], vertices[4], vertices[5]);
		vec3 const c(vertices[9], vertices[10], vertices[11]);
		vec3 const tangent = b - a;
		vec3 const binormal = c - a;
		vec3 const normal = cross(tangent, binormal); // No need to normalize, edges are 1.

		mat3 const rotation(tangent, binormal, normal);

		// TODO We can avoid casting duplicate rays for adjacent quads by
		// casting and caching them for each of the 8 'quadrants' separately.
		for (unsigned j = 0; j < 4; ++j) {
			vec3 const vertex(vertices[0], vertices[1], vertices[2]);

			vec3 bentNormal(0, 0, 0);
			vec3 sum(0, 0, 0);
			unsigned const THETA_STEPS = 3;
			unsigned const PHI_STEPS = 4;
			for (unsigned t = 1; t <= THETA_STEPS; ++t) {
				float const theta = 0.5f * M_PI * t / (THETA_STEPS + 1);
				for (unsigned p = 0; p < PHI_STEPS; ++p) {
					float const phi = 2.0f * M_PI * (p + (t % 2 == 0 ? 0.5f : 0.0f)) / PHI_STEPS;
					float const cosTheta = cosf(theta);
					float const sinTheta = sinf(theta);
					float const cosPhi = cosf(phi);
					float const sinPhi = sinf(phi);
					vec3 const direction = rotation * vec3(
						cosTheta * cosPhi,
						cosTheta * sinPhi,
						sinTheta);
					sum += direction;

					RaycastResult result = raycast(index, vertex + 0.1f * direction, direction);
					float factor = 1.0f;
					if (result.status == RaycastResult::HIT) {
						factor = result.length / cutoff;
					}
					bentNormal += factor * direction;
				}
			}
			bentNormal /= length(sum);

			normals[0] = (int)(0x7F * bentNormal.x);
			normals[1] = (int)(0x7F * bentNormal.y);
			normals[2] = (int)(0x7F * bentNormal.z);

			vertices += 3;
			normals += 3;
		}
	}

	stats.quadsLit.increment(numQuads);
	stats.chunksLit.increment();
}
