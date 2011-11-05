#ifndef LIGHTING_H
#define LIGHTING_H

#include "maths.h"

class Sun {

	float const season;
	float const latitude;
	float const axialTilt;

	public:

		Sun(float season, float latitude, float axialTilt);

		vec3 directionAtTime(float timeOfDay);

};

class Lighting {

	float wallTime;
	float timeOfDay;
	float const dayLength;
	Sun sun;

	public:

		Lighting(float startTime, float dayLength, Sun sun);

		void update(float dt);
		void setup();

};

#endif
