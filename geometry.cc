#include "geometry.h"

#include "chunkdata.h"
#include "chunkmap.h"
#include "flags.h"
#include "raycaster.h"
#include "stats.h"

class Tesselator {

	ChunkMap const &chunkMap;
	
	int3 index;
	ChunkGeometryPtr geometry;
	VertexArray *vertices;
	NormalArray *normals;

	RawChunkData rawData;
	RawChunkData rawNeighData; // Reused to save allocations

	public:

		Tesselator(ChunkMap const &chunkMap)
		:
			chunkMap(chunkMap)
		{
		}

		void tesselate(int3 index, ChunkGeometryPtr geometry) {
			TimerStat::Timed t = stats.chunkTesselationTime.timed();

			this->index = index;
			this->geometry = geometry;
			vertices = &geometry->getVertexData();
			normals = &geometry->getNormalData();
			vertices->clear();
			normals->clear();

			OctreeConstPtr octree = chunkMap.getOctreeOrNull(index);
			if (octree && !octree->isEmpty()) {
				unpackOctree(*octree, rawData);

				tesselateDirection<-1,  0,  0>();
				tesselateDirection< 1,  0,  0>();
				tesselateDirection< 0, -1,  0>();
				tesselateDirection< 0,  1,  0>();
				tesselateDirection< 0,  0, -1>();
				tesselateDirection< 0,  0,  1>();
			}

			stats.chunksTesselated.increment();
			// TODO stat for quads generated
		}

	private:

		static short const CUBE_FACES[6][12];

		template<int dx, int dy, int dz>
		struct FaceIndex {
			static int value;
		};

		template<int dx, int dy, int dz>
		std::vector<vec3> computeRaycastDirections() {
			static short const *face = CUBE_FACES[FaceIndex<dx, dy, dz>::value];
			vec3 const a(face[0], face[1], face[2]);
			vec3 const b(face[3], face[4], face[5]);
			vec3 const c(face[9], face[10], face[11]);
			vec3 const tangent = b - a;
			vec3 const binormal = c - a;
			vec3 const normal = cross(tangent, binormal); // No need to normalize, edges are 1.

			mat3 const rotation(tangent, binormal, normal);

			// TODO We can avoid casting duplicate rays for adjacent quads by
			// casting and caching them for each of the 8 'quadrants' separately.
			std::vector<vec3> raycastDirections;

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
					raycastDirections.push_back(direction);
				}
			}

			return raycastDirections;
		}

		template<int dx, int dy, int dz>
		inline vec3 computeBentNormal(vec3 vertex) {
			static std::vector<vec3> const raycastDirections = computeRaycastDirections<dx, dy, dz>();

			float const cutoff = CHUNK_SIZE;
			Raycaster raycast(chunkMap, cutoff, STONE_BLOCK, BLOCK_MASK);
			vec3 bentNormal(0, 0, 0);
			vec3 sum(0, 0, 0);
			for (unsigned i = 0; i < raycastDirections.size(); ++i) {
				vec3 const direction = raycastDirections[i];
				sum += direction;

				RaycastResult result = raycast(index, vertex + 0.1f * direction, direction);
				float factor = 1.0f;
				if (result.status == RaycastResult::HIT) {
					factor = result.length / cutoff;
				}
				bentNormal += factor * direction;
			}
			return bentNormal / length(sum);
		}

		template<int dx, int dy, int dz>
		inline void tesselateSingleBlockFace(
				Block block, Block neigh,
				unsigned x, unsigned y, unsigned z,
				unsigned &end)
		{
			static char const N = 0x7F;
			static char const n[12] = {
				dx * N, dy * N, dz * N,
				dx * N, dy * N, dz * N,
				dx * N, dy * N, dz * N,
				dx * N, dy * N, dz * N,
			};
			static short const *face = CUBE_FACES[FaceIndex<dx, dy, dz>::value];

			if (needsDrawing(block) && needsDrawing(block, neigh)) {
				int3 const pos(x, y, z);
				int3 m = (int3)blockMin(pos);
				short v[12] = {
					face[0] + m.x, face[ 1] + m.y, face[ 2] + m.z,
					face[3] + m.x, face[ 4] + m.y, face[ 5] + m.z,
					face[6] + m.x, face[ 7] + m.y, face[ 8] + m.z,
					face[9] + m.x, face[10] + m.y, face[11] + m.z
				};

				vertices->resize(end + 12);
				normals->resize(end + 12);
				memcpy(&(*vertices)[end], v, 12 * sizeof(short));
				if (flags.bentNormals) {
					for (unsigned j = 0; j < 4; ++j) {
						// TODO try not to convert from short
						vec3 const vertex((*vertices)[end], (*vertices)[end + 1], (*vertices)[end + 2]);
						vec3 normal = computeBentNormal<dx, dy, dz>(vertex);
						(*normals)[end++] = (int)(N * normal.x);
						(*normals)[end++] = (int)(N * normal.y);
						(*normals)[end++] = (int)(N * normal.z);
					}
				} else {
					memcpy(&(*normals)[end], n, 12 * sizeof(char));
					end += 12;
				}
			}
		}

		template<int dx, int dy, int dz>
		inline void tesselateNeigh(Block const *rawData, Block const *rawNeighData, unsigned &end);

		template<int dx, int dy, int dz>
		void tesselateDirection() {
			static int const neighOffset = dx + (int)CHUNK_SIZE * dy + (int)CHUNK_SIZE * (int)CHUNK_SIZE * dz;

			unsigned const begin = vertices->size();

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
						tesselateSingleBlockFace<dx, dy, dz>(*p, p[neighOffset], x, y, z, end);
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
			tesselateNeigh<dx, dy, dz>(raw, rawNeighData.raw(), end);

			stats.quadsGenerated.increment(vertices->size() / 4);

			geometry->setRange(FaceIndex<dx, dy, dz>::value, Range(begin, end));
		}

};

short const Tesselator::CUBE_FACES[6][12] = {
	{ 0, 0, 0, 0, 0, 1, 0, 1, 1, 0, 1, 0 },
	{ 1, 0, 0, 1, 1, 0, 1, 1, 1, 1, 0, 1 },
	{ 0, 0, 0, 1, 0, 0, 1, 0, 1, 0, 0, 1 },
	{ 0, 1, 0, 0, 1, 1, 1, 1, 1, 1, 1, 0 },
	{ 0, 0, 0, 0, 1, 0, 1, 1, 0, 1, 0, 0 },
	{ 0, 0, 1, 1, 0, 1, 1, 1, 1, 0, 1, 1 }
};

template<> int Tesselator::FaceIndex<-1,  0,  0>::value = 0;
template<> int Tesselator::FaceIndex< 1,  0,  0>::value = 1;
template<> int Tesselator::FaceIndex< 0, -1,  0>::value = 2;
template<> int Tesselator::FaceIndex< 0,  1,  0>::value = 3;
template<> int Tesselator::FaceIndex< 0,  0, -1>::value = 4;
template<> int Tesselator::FaceIndex< 0,  0,  1>::value = 5;

template<>
inline void Tesselator::tesselateNeigh<-1, 0, 0>(Block const *p, Block const *q, unsigned &end) {
	q += CHUNK_SIZE - 1;
	for (unsigned z = 0; z < CHUNK_SIZE; ++z) {
		for (unsigned y = 0; y < CHUNK_SIZE; ++y) {
			tesselateSingleBlockFace<-1, 0, 0>(*p, *q, (unsigned)0, y, z, end);
			p += CHUNK_SIZE;
			q += CHUNK_SIZE;
		}
	}
}

template<>
inline void Tesselator::tesselateNeigh<1, 0, 0>(Block const *p, Block const *q, unsigned &end) {
	p += CHUNK_SIZE - 1;
	for (unsigned z = 0; z < CHUNK_SIZE; ++z) {
		for (unsigned y = 0; y < CHUNK_SIZE; ++y) {
			tesselateSingleBlockFace<1, 0, 0>(*p, *q, CHUNK_SIZE - 1, y, z, end);
			p += CHUNK_SIZE;
			q += CHUNK_SIZE;
		}
	}
}

template<>
inline void Tesselator::tesselateNeigh<0, -1, 0>(Block const *p, Block const *q, unsigned &end) {
	q += CHUNK_SIZE * (CHUNK_SIZE - 1);
	for (unsigned z = 0; z < CHUNK_SIZE; ++z) {
		for (unsigned x = 0; x < CHUNK_SIZE; ++x) {
			tesselateSingleBlockFace<0, -1, 0>(*p, *q, x, (unsigned)0, z, end);
			++p;
			++q;
		}
		p += CHUNK_SIZE * (CHUNK_SIZE - 1);
		q += CHUNK_SIZE * (CHUNK_SIZE - 1);
	}
}

template<>
inline void Tesselator::tesselateNeigh<0, 1, 0>(Block const *p, Block const *q, unsigned &end) {
	p += CHUNK_SIZE * (CHUNK_SIZE - 1);
	for (unsigned z = 0; z < CHUNK_SIZE; ++z) {
		for (unsigned x = 0; x < CHUNK_SIZE; ++x) {
			tesselateSingleBlockFace<0, 1, 0>(*p, *q, x, CHUNK_SIZE - 1, z, end);
			++p;
			++q;
		}
		p += CHUNK_SIZE * (CHUNK_SIZE - 1);
		q += CHUNK_SIZE * (CHUNK_SIZE - 1);
	}
}

template<>
inline void Tesselator::tesselateNeigh<0, 0, -1>(Block const *p, Block const *q, unsigned &end) {
	q += CHUNK_SIZE * CHUNK_SIZE * (CHUNK_SIZE - 1);
	for (unsigned y = 0; y < CHUNK_SIZE; ++y) {
		for (unsigned x = 0; x < CHUNK_SIZE; ++x) {
			tesselateSingleBlockFace<0, 0, -1>(*p, *q, x, y, (unsigned)0, end);
			++p;
			++q;
		}
	}
}

template<>
inline void Tesselator::tesselateNeigh<0, 0, 1>(Block const *p, Block const *q, unsigned &end) {
	p += CHUNK_SIZE * CHUNK_SIZE * (CHUNK_SIZE - 1);
	for (unsigned y = 0; y < CHUNK_SIZE; ++y) {
		for (unsigned x = 0; x < CHUNK_SIZE; ++x) {
			tesselateSingleBlockFace<0, 0, 1>(*p, *q, x, y, CHUNK_SIZE - 1, end);
			++p;
			++q;
		}
	}
}

void tesselate(int3 index, ChunkMap const &chunkMap, ChunkGeometryPtr geometry) {
	Tesselator tesselator(chunkMap);
	tesselator.tesselate(index, geometry);
}
