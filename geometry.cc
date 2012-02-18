#include "geometry.h"

#include "chunkdata.h"
#include "chunkmap.h"
#include "raycaster.h"
#include "stats.h"

template<int dx, int dy, int dz>
struct FaceIndex {
	static int value;
};

template<> int FaceIndex<-1,  0,  0>::value = 0;
template<> int FaceIndex< 1,  0,  0>::value = 1;
template<> int FaceIndex< 0, -1,  0>::value = 2;
template<> int FaceIndex< 0,  1,  0>::value = 3;
template<> int FaceIndex< 0,  0, -1>::value = 4;
template<> int FaceIndex< 0,  0,  1>::value = 5;

template<int dx, int dy, int dz>
inline void tesselateSingleBlockFace(
		Block block, Block neigh,
		unsigned x, unsigned y, unsigned z,
		VertexArray &vertices, NormalArray &normals, unsigned &end)
{
	static char const N = 0x7F;
	static char const n[12] = {
		dx * N, dy * N, dz * N,
		dx * N, dy * N, dz * N,
		dx * N, dy * N, dz * N,
		dx * N, dy * N, dz * N,
	};
	static short const faces[6][12] = {
		{ 0, 0, 0, 0, 0, 1, 0, 1, 1, 0, 1, 0 },
		{ 1, 0, 0, 1, 1, 0, 1, 1, 1, 1, 0, 1 },
		{ 0, 0, 0, 1, 0, 0, 1, 0, 1, 0, 0, 1 },
		{ 0, 1, 0, 0, 1, 1, 1, 1, 1, 1, 1, 0 },
		{ 0, 0, 0, 0, 1, 0, 1, 1, 0, 1, 0, 0 },
		{ 0, 0, 1, 1, 0, 1, 1, 1, 1, 0, 1, 1 }
	};
	static short const *face = faces[FaceIndex<dx, dy, dz>::value];

	if (needsDrawing(block) && needsDrawing(block, neigh)) {
		int3 const pos(x, y, z);
		int3 m = (int3)blockMin(pos);
		short v[12] = {
			face[0] + m.x, face[ 1] + m.y, face[ 2] + m.z,
			face[3] + m.x, face[ 4] + m.y, face[ 5] + m.z,
			face[6] + m.x, face[ 7] + m.y, face[ 8] + m.z,
			face[9] + m.x, face[10] + m.y, face[11] + m.z
		};
		vertices.resize(end + 12);
		normals.resize(end + 12);
		memcpy(&vertices[end], v, 12 * sizeof(short));
		memcpy(&normals[end], n, 12 * sizeof(char));
		end += 12;
	}
}

template<int dx, int dy, int dz>
inline void tesselateNeigh(Block const *rawData, Block const *rawNeighData, VertexArray &vertices, NormalArray &normals, unsigned &end);

template<>
inline void tesselateNeigh<-1, 0, 0>(Block const *p, Block const *q, VertexArray &vertices, NormalArray &normals, unsigned &end) {
	q += CHUNK_SIZE - 1;
	for (unsigned z = 0; z < CHUNK_SIZE; ++z) {
		for (unsigned y = 0; y < CHUNK_SIZE; ++y) {
			tesselateSingleBlockFace<-1, 0, 0>(*p, *q, (unsigned)0, y, z, vertices, normals, end);
			p += CHUNK_SIZE;
			q += CHUNK_SIZE;
		}
	}
}

template<>
inline void tesselateNeigh<1, 0, 0>(Block const *p, Block const *q, VertexArray &vertices, NormalArray &normals, unsigned &end) {
	p += CHUNK_SIZE - 1;
	for (unsigned z = 0; z < CHUNK_SIZE; ++z) {
		for (unsigned y = 0; y < CHUNK_SIZE; ++y) {
			tesselateSingleBlockFace<1, 0, 0>(*p, *q, CHUNK_SIZE - 1, y, z, vertices, normals, end);
			p += CHUNK_SIZE;
			q += CHUNK_SIZE;
		}
	}
}

template<>
inline void tesselateNeigh<0, -1, 0>(Block const *p, Block const *q, VertexArray &vertices, NormalArray &normals, unsigned &end) {
	q += CHUNK_SIZE * (CHUNK_SIZE - 1);
	for (unsigned z = 0; z < CHUNK_SIZE; ++z) {
		for (unsigned x = 0; x < CHUNK_SIZE; ++x) {
			tesselateSingleBlockFace<0, -1, 0>(*p, *q, x, (unsigned)0, z, vertices, normals, end);
			++p;
			++q;
		}
		p += CHUNK_SIZE * (CHUNK_SIZE - 1);
		q += CHUNK_SIZE * (CHUNK_SIZE - 1);
	}
}

template<>
inline void tesselateNeigh<0, 1, 0>(Block const *p, Block const *q, VertexArray &vertices, NormalArray &normals, unsigned &end) {
	p += CHUNK_SIZE * (CHUNK_SIZE - 1);
	for (unsigned z = 0; z < CHUNK_SIZE; ++z) {
		for (unsigned x = 0; x < CHUNK_SIZE; ++x) {
			tesselateSingleBlockFace<0, 1, 0>(*p, *q, x, CHUNK_SIZE - 1, z, vertices, normals, end);
			++p;
			++q;
		}
		p += CHUNK_SIZE * (CHUNK_SIZE - 1);
		q += CHUNK_SIZE * (CHUNK_SIZE - 1);
	}
}

template<>
inline void tesselateNeigh<0, 0, -1>(Block const *p, Block const *q, VertexArray &vertices, NormalArray &normals, unsigned &end) {
	q += CHUNK_SIZE * CHUNK_SIZE * (CHUNK_SIZE - 1);
	for (unsigned y = 0; y < CHUNK_SIZE; ++y) {
		for (unsigned x = 0; x < CHUNK_SIZE; ++x) {
			tesselateSingleBlockFace<0, 0, -1>(*p, *q, x, y, (unsigned)0, vertices, normals, end);
			++p;
			++q;
		}
	}
}

template<>
inline void tesselateNeigh<0, 0, 1>(Block const *p, Block const *q, VertexArray &vertices, NormalArray &normals, unsigned &end) {
	p += CHUNK_SIZE * CHUNK_SIZE * (CHUNK_SIZE - 1);
	for (unsigned y = 0; y < CHUNK_SIZE; ++y) {
		for (unsigned x = 0; x < CHUNK_SIZE; ++x) {
			tesselateSingleBlockFace<0, 0, 1>(*p, *q, x, y, CHUNK_SIZE - 1, vertices, normals, end);
			++p;
			++q;
		}
	}
}

template<int dx, int dy, int dz>
void tesselateDirection(RawChunkData &rawData, int3 index, ChunkMap const &chunkMap, RawChunkData &rawNeighData, ChunkGeometryPtr geometry) {
	static int const neighOffset = dx + (int)CHUNK_SIZE * dy + (int)CHUNK_SIZE * (int)CHUNK_SIZE * dz;

	VertexArray &vertices = geometry->getVertexData();
	NormalArray &normals = geometry->getNormalData();

	unsigned const begin = vertices.size();

	unsigned end = begin;
	unsigned const xMin = (dx == -1 ? 1 : 0);
	unsigned const yMin = (dy == -1 ? 1 : 0);
	unsigned const zMin = (dz == -1 ? 1 : 0);
	unsigned const xMax = CHUNK_SIZE - (dx == 1 ? 1 : 0);
	unsigned const yMax = CHUNK_SIZE - (dy == 1 ? 1 : 0);
	unsigned const zMax = CHUNK_SIZE - (dz == 1 ? 1 : 0);
	Block const *raw = rawData.raw();
	Block const *p = raw +
		xMin +
		yMin * CHUNK_SIZE +
		zMin * CHUNK_SIZE * CHUNK_SIZE;
	for (unsigned z = zMin; z < zMax; ++z) {
		for (unsigned y = yMin; y < yMax; ++y) {
			for (unsigned x = xMin; x < xMax; ++x) {
				tesselateSingleBlockFace<dx, dy, dz>(*p, p[neighOffset], x, y, z, vertices, normals, end);
				++p;
			}
			if (dx != 0) {
				++p;
			}
		}
		if (dy != 0) {
			p += CHUNK_SIZE;
		}
	}

	OctreeConstPtr neighOctree = chunkMap.getOctreeOrNull(index + int3(dx, dy, dz));
	BOOST_ASSERT(neighOctree);
	unpackOctree(*neighOctree, rawNeighData);
	tesselateNeigh<dx, dy, dz>(raw, rawNeighData.raw(), vertices, normals, end);

	stats.quadsGenerated.increment(vertices.size() / 4);

	geometry->setRange(FaceIndex<dx, dy, dz>::value, Range(begin, end));
}

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

void tesselate(int3 index, ChunkMap const &chunkMap, ChunkGeometryPtr geometry) {
	TimerStat::Timed t = stats.chunkTesselationTime.timed();

	OctreeConstPtr octree = chunkMap.getOctreeOrNull(index);
	if (octree && !octree->isEmpty()) {
		RawChunkData rawData;
		unpackOctree(*octree, rawData);

		RawChunkData rawNeighData;
		tesselateDirection<-1,  0,  0>(rawData, index, chunkMap, rawNeighData, geometry);
		tesselateDirection< 1,  0,  0>(rawData, index, chunkMap, rawNeighData, geometry);
		tesselateDirection< 0, -1,  0>(rawData, index, chunkMap, rawNeighData, geometry);
		tesselateDirection< 0,  1,  0>(rawData, index, chunkMap, rawNeighData, geometry);
		tesselateDirection< 0,  0, -1>(rawData, index, chunkMap, rawNeighData, geometry);
		tesselateDirection< 0,  0,  1>(rawData, index, chunkMap, rawNeighData, geometry);
	}

	stats.chunksTesselated.increment();

	// TODO integrate into tesselation procedure
	computeLighting(index, chunkMap, geometry);
}
