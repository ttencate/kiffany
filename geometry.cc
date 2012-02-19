#include "geometry.h"

#include "chunkdata.h"
#include "chunkmap.h"
#include "flags.h"
#include "raycaster.h"
#include "stats.h"

class Tesselator {

	std::vector<vec3> raycastDirections[8];
	Raycaster raycast;

	ChunkMap const &chunkMap;
	
	int3 index;
	ChunkGeometryPtr geometry;
	VertexArray *vertices;
	NormalArray *normals;

	RawChunkData rawData;
	RawChunkData rawNeighData;

	public:

		Tesselator(ChunkMap const &chunkMap, float raycastCutoff = CHUNK_SIZE)
		:
			raycast(chunkMap, raycastCutoff, STONE_BLOCK, BLOCK_MASK),
			chunkMap(chunkMap)
		{
			computeRaycastDirections();
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
			stats.quadsGenerated.increment(vertices->size() / 4);
		}

	private:

		static short const CUBE_FACES[6][12];

		template<int dx, int dy, int dz>
		struct FaceIndex {
			static int value;
		};

		void computeRaycastDirections() {
			std::vector<vec3> directions;
			// TODO fiddle with the values -- theoretical optimum is not clear ATM
			vec3 const d = normalize(vec3(1.0f, 0.5f, 0.5f));
			directions.push_back(d);
			directions.push_back(vec3(d.z, d.x, d.y));
			directions.push_back(vec3(d.y, d.z, d.x));

			unsigned index = 0;
			for (int z = -1; z <= 1; z += 2) {
				for (int y = -1; y <= 1; y += 2) {
					for (int x = -1; x <= 1; x += 2) {
						vec3 const mirror = vec3(x, y, z);
						for (unsigned i = 0; i < directions.size(); ++i) {
							raycastDirections[index].push_back(directions[i] * mirror);
						}
						++index;
					}
				}
			}
		}

		template<int dx, int dy, int dz>
		inline vec3 computeBentNormal(vec3 vertex) {
			static unsigned raycastDirectionIndicesTable[6][4] = {
				{ 0, 2, 4, 6 },
				{ 1, 3, 5, 7 },
				{ 0, 1, 4, 5 },
				{ 2, 3, 6, 7 },
				{ 0, 1, 2, 3 },
				{ 4, 5, 6, 7 },
			};
			static unsigned *raycastDirectionIndices = raycastDirectionIndicesTable[FaceIndex<dx, dy, dz>::value];

			vec3 bentNormal(0, 0, 0);
			vec3 sum(0, 0, 0);
			for (unsigned i = 0; i < 4; ++i) {
				std::vector<vec3> const &raycastDirections = this->raycastDirections[raycastDirectionIndices[i]];
				for (unsigned j = 0; j < raycastDirections.size(); ++j) {
					vec3 const direction = raycastDirections[j];
					sum += direction;

					RaycastResult result = raycast(index, vertex + 0.1f * direction, direction);
					float factor = 1.0f;
					if (result.status == RaycastResult::HIT) {
						factor = result.length / raycast.getCutoff();
					}
					bentNormal += factor * direction;
				}
			}
			return bentNormal / length(sum);
		}

		template<int dx, int dy, int dz>
		inline void tesselateSingleBlockFace(Block block, Block neigh, unsigned x, unsigned y, unsigned z) {
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

				unsigned writeIndex = vertices->size();
				vertices->resize(writeIndex + 12);
				normals->resize(writeIndex + 12);

				memcpy(&(*vertices)[writeIndex], v, 12 * sizeof(short));
				if (flags.bentNormals) {
					for (unsigned j = 0; j < 4; ++j) {
						// TODO try not to convert from short
						vec3 const vertex((*vertices)[writeIndex], (*vertices)[writeIndex + 1], (*vertices)[writeIndex + 2]);
						vec3 normal = computeBentNormal<dx, dy, dz>(vertex);
						(*normals)[writeIndex++] = (int)(N * normal.x);
						(*normals)[writeIndex++] = (int)(N * normal.y);
						(*normals)[writeIndex++] = (int)(N * normal.z);
					}
				} else {
					memcpy(&(*normals)[writeIndex], n, 12 * sizeof(char));
				}
			}
		}

		template<int dx, int dy, int dz>
		inline void tesselateNeigh(Block const *rawData, Block const *rawNeighData);

		template<int dx, int dy, int dz>
		void tesselateDirection() {
			static int const neighOffset = dx + (int)CHUNK_SIZE * dy + (int)CHUNK_SIZE * (int)CHUNK_SIZE * dz;

			unsigned const begin = vertices->size();

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
						tesselateSingleBlockFace<dx, dy, dz>(*p, p[neighOffset], x, y, z);
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
			tesselateNeigh<dx, dy, dz>(raw, rawNeighData.raw());

			unsigned const end = vertices->size();
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
inline void Tesselator::tesselateNeigh<-1, 0, 0>(Block const *p, Block const *q) {
	q += CHUNK_SIZE - 1;
	for (unsigned z = 0; z < CHUNK_SIZE; ++z) {
		for (unsigned y = 0; y < CHUNK_SIZE; ++y) {
			tesselateSingleBlockFace<-1, 0, 0>(*p, *q, (unsigned)0, y, z);
			p += CHUNK_SIZE;
			q += CHUNK_SIZE;
		}
	}
}

template<>
inline void Tesselator::tesselateNeigh<1, 0, 0>(Block const *p, Block const *q) {
	p += CHUNK_SIZE - 1;
	for (unsigned z = 0; z < CHUNK_SIZE; ++z) {
		for (unsigned y = 0; y < CHUNK_SIZE; ++y) {
			tesselateSingleBlockFace<1, 0, 0>(*p, *q, CHUNK_SIZE - 1, y, z);
			p += CHUNK_SIZE;
			q += CHUNK_SIZE;
		}
	}
}

template<>
inline void Tesselator::tesselateNeigh<0, -1, 0>(Block const *p, Block const *q) {
	q += CHUNK_SIZE * (CHUNK_SIZE - 1);
	for (unsigned z = 0; z < CHUNK_SIZE; ++z) {
		for (unsigned x = 0; x < CHUNK_SIZE; ++x) {
			tesselateSingleBlockFace<0, -1, 0>(*p, *q, x, (unsigned)0, z);
			++p;
			++q;
		}
		p += CHUNK_SIZE * (CHUNK_SIZE - 1);
		q += CHUNK_SIZE * (CHUNK_SIZE - 1);
	}
}

template<>
inline void Tesselator::tesselateNeigh<0, 1, 0>(Block const *p, Block const *q) {
	p += CHUNK_SIZE * (CHUNK_SIZE - 1);
	for (unsigned z = 0; z < CHUNK_SIZE; ++z) {
		for (unsigned x = 0; x < CHUNK_SIZE; ++x) {
			tesselateSingleBlockFace<0, 1, 0>(*p, *q, x, CHUNK_SIZE - 1, z);
			++p;
			++q;
		}
		p += CHUNK_SIZE * (CHUNK_SIZE - 1);
		q += CHUNK_SIZE * (CHUNK_SIZE - 1);
	}
}

template<>
inline void Tesselator::tesselateNeigh<0, 0, -1>(Block const *p, Block const *q) {
	q += CHUNK_SIZE * CHUNK_SIZE * (CHUNK_SIZE - 1);
	for (unsigned y = 0; y < CHUNK_SIZE; ++y) {
		for (unsigned x = 0; x < CHUNK_SIZE; ++x) {
			tesselateSingleBlockFace<0, 0, -1>(*p, *q, x, y, (unsigned)0);
			++p;
			++q;
		}
	}
}

template<>
inline void Tesselator::tesselateNeigh<0, 0, 1>(Block const *p, Block const *q) {
	p += CHUNK_SIZE * CHUNK_SIZE * (CHUNK_SIZE - 1);
	for (unsigned y = 0; y < CHUNK_SIZE; ++y) {
		for (unsigned x = 0; x < CHUNK_SIZE; ++x) {
			tesselateSingleBlockFace<0, 0, 1>(*p, *q, x, y, CHUNK_SIZE - 1);
			++p;
			++q;
		}
	}
}

void tesselate(int3 index, ChunkMap const &chunkMap, ChunkGeometryPtr geometry) {
	Tesselator tesselator(chunkMap);
	tesselator.tesselate(index, geometry);
}
