#ifndef WORLD_H
#define WORLD_H

#include "camera.h"
#include "terrain.h"

class World {

	Camera *camera;
	Terrain terrain;

	public:

		World(Camera *camera);

		void render() const;

};

#endif
