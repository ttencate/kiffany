#ifndef TERRAIN_H
#define TERRAIN_H

#include "chunkmanager.h"
#include "chunkmap.h"
#include "shader.h"

#include <boost/noncopyable.hpp>

class Camera;
class Lighting;
class TerrainGenerator;

class Terrain
:
	boost::noncopyable
{

	ChunkMap chunkMap;
	ChunkManager chunkManager;

	ShaderProgram shaderProgram;

	public:

		Terrain(TerrainGenerator *terrainGenerator);
		~Terrain();

		ChunkMap const &getChunkMap() const { return chunkMap; }

		void addViewSphere(WeakConstViewSpherePtr viewSphere);

		void update(float dt);
		void render(Camera const &camera, Lighting const &lighting);

	private:

		unsigned computeMaxNumChunks() const;
		void renderChunk(Camera const &camera, int3 const &index);

};

#endif
