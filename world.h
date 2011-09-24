#ifndef WORLD_H
#define WORLD_H

#include "terrain.h"

class Camera;
class TerrainGenerator;

class World {

	Camera *camera;
	Terrain terrain;

	public:

		World(Camera *camera, TerrainGenerator *terrainGenerator);

		void render();

};

#endif
