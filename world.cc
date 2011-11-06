#include "world.h"

World::World(Camera *camera, TerrainGenerator *terrainGenerator, Lighting *lighting, Sky *sky)
:
	camera(camera),
	terrain(terrainGenerator),
	lighting(lighting),
	sky(sky)
{
}

void World::update(float dt) {
	terrain.update(dt);
	sky->update(dt);
	lighting->update(dt);
}

void World::render() {
	glMatrixMode(GL_MODELVIEW);
	glLoadMatrixf(value_ptr(camera->getRotationMatrix()));

	sky->render();

	glLoadMatrixf(value_ptr(camera->getViewMatrix()));

	lighting->setup();
	terrain.render(*camera);
}
