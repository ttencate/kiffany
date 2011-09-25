#include "world.h"

World::World(Camera *camera, TerrainGenerator *terrainGenerator)
:
	camera(camera),
	terrain(terrainGenerator)
{
}

void World::update(float dt) {
	terrain.update(dt);
}

void World::render() {
	terrain.render(*camera);
}
