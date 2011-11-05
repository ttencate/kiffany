#include "lighting.h"

#include "gl.h"

Sun::Sun(float season, float latitude, float axialTilt)
:
	season(season),
	latitude(latitude),
	axialTilt(axialTilt)
{
}

vec3 Sun::directionAtTime(float timeOfDay) {
	// TODO use parameters to return the real value
	float a = 2.0f * M_PI * timeOfDay;
	return vec3(
		sinf(a),
		0.0f,
		-cosf(a));
}

Lighting::Lighting(float startTime, float dayLength, Sun sun)
:
	timeOfDay(startTime),
	dayLength(dayLength),
	sun(sun)
{
}

void Lighting::update(float dt) {
	if (dayLength > 0) {
		timeOfDay += dt / dayLength;
		while (timeOfDay >= 1.0f) {
			timeOfDay -= 1.0f;
		}
	}
}

void Lighting::setup() {
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	float globalAmbient[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, globalAmbient);

	vec3 sunDirection = sun.directionAtTime(timeOfDay);

	float const af = sunDirection.z;
	float const ambientLight[] = { af * 0.45f, af * 0.5f, af * 0.55f, 1.0f };

	float const df = smoothstep(0.0f, 0.3f, sunDirection.z);
	float diffuseLight[] = { df * 1.0f, df * 0.95f, df * 0.9f, 1.0f };

	float lightPos[] = { sunDirection.x, sunDirection.y, sunDirection.z, 0.0f };
	glLightfv(GL_LIGHT0, GL_AMBIENT, ambientLight);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuseLight);
	glLightfv(GL_LIGHT0, GL_POSITION, lightPos);
}
