#ifndef SPACE_H
#define SPACE_H

#include "maths.h"

#include <boost/shared_ptr.hpp>

class Sun {

	float const season;
	float const latitude;
	float const axialTilt;
	float const dayLength;

	float wallTime;
	float timeOfDay;

	vec3 direction;
	vec3 color;

	vec3 directionAtTime(float timeOfDay) const;

	public:

		Sun(float season, float latitude, float axialTilt, float timeOfDay, float dayLength);

		vec3 getDirection() const { return direction; }
		vec3 getColor() const { return color; }

		void update(float dt);

};

#endif
