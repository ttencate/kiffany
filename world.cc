#include "world.h"

World::World(Camera *camera, TerrainGenerator *terrainGenerator, Lighting *lighting)
:
	camera(camera),
	terrain(terrainGenerator),
	lighting(lighting)
{
}

void World::update(float dt) {
	terrain.update(dt);
	lighting->update(dt);
}

void World::render() {
	lighting->setup();
	terrain.render(*camera);
}
