#include "world.h"

#include "flags.h"

World::World(Camera *camera, TerrainGenerator *terrainGenerator, Sun *sun, Lighting *lighting, Sky *sky)
:
	camera(camera),
	terrain(terrainGenerator),
	sun(sun),
	lighting(lighting),
	sky(sky),
	viewSphere(new ViewSphere(camera->getPosition(), flags.viewDistance))
{
	terrain.addViewSphere(viewSphere);
}

void World::update(float dt) {
	viewSphere->center = camera->getPosition();
	terrain.update(dt);
	sky->update(dt);
	sun->update(dt);
}

void World::render() {
	glMatrixMode(GL_MODELVIEW);
	glLoadMatrixf(value_ptr(camera->getRotationMatrix()));

	sky->render();

	glLoadMatrixf(value_ptr(camera->getViewMatrix()));

	lighting->setup();
	terrain.render(*camera);
}
