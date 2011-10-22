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

		Terrain &getTerrain() { return terrain; }
		Terrain const &getTerrain() const { return terrain; }

		void update(float dt);
		void render();

};

#endif
