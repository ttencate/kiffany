#ifndef SPACE_H
#define SPACE_H

#include "maths.h"

#include <boost/shared_ptr.hpp>

class Sun {

	float const season;
	float const latitude;
	float const axialTilt;
	float const dayLength;
	float const angularRadius;
	float const brightness;
	vec3 const color;

	float timeOfDay;

	vec3 direction;

	vec3 directionAtTime(float timeOfDay) const;

	public:

		Sun(float season, float latitude, float axialTilt, float dayLength, float angularRadius, float brightness, vec3 color, float timeOfDay);

		float getAngularRadius() const { return angularRadius; }
		vec3 getColor() const { return color; }
		vec3 getScaledColor() const { return brightness * color; }

		vec3 getDirection() const { return direction; }

		void update(float dt);

};

#endif
