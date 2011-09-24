#ifndef WORLD_H
#define WORLD_H

#include "terrain.h"

class World {

	Terrain terrain;

	public:

		World();

		void render() const;

};

#endif
