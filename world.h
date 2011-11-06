#ifndef WORLD_H
#define WORLD_H

#include "lighting.h"
#include "sky.h"
#include "terrain.h"

#include <boost/scoped_ptr.hpp>

class Camera;
class TerrainGenerator;

class World {

	Camera *camera;
	Terrain terrain;
	boost::scoped_ptr<Lighting> lighting;
	boost::scoped_ptr<Sky> sky;

	public:

		World(Camera *camera, TerrainGenerator *terrainGenerator, Lighting *lighting, Sky *sky);

		Terrain &getTerrain() { return terrain; }
		Terrain const &getTerrain() const { return terrain; }

		void update(float dt);
		void render();

};

#endif
