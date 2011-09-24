#include "world.h"

World::World(Camera *camera)
:
	camera(camera)
{
}

void World::render() const {
	terrain.render();
}
