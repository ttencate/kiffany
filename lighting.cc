#include "lighting.h"

#include "gl.h"

void Lighting::update(float dt) {
}

void Lighting::setup() {
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	float globalAmbient[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, globalAmbient);

	float ambientLight[] = { 0.45f, 0.5f, 0.55f, 1.0f };
	float diffuseLight[] = { 1.0f, 0.95f, 0.9f, 1.0f };
	float lightPos[] = { 1.0f, 2.0f, 3.0f, 0.0f };
	glLightfv(GL_LIGHT0, GL_AMBIENT, ambientLight);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuseLight);
	glLightfv(GL_LIGHT0, GL_POSITION, lightPos);
}
