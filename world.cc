#include "world.h"

World::World(Camera *camera, TerrainGenerator *terrainGenerator)
:
	camera(camera),
	terrain(terrainGenerator)
{
}

void World::render() {
	terrain.render(*camera);
}
