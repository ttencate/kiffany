#include "space.h"

Sun::Sun(float season, float latitude, float axialTilt, float timeOfDay, float dayLength)
:
	season(season),
	latitude(latitude),
	axialTilt(axialTilt),
	dayLength(dayLength),
	timeOfDay(timeOfDay),
	color(1.0f, 1.0f, 1.0f)
{
	direction = directionAtTime(timeOfDay);
}

vec3 Sun::directionAtTime(float timeOfDay) const {
	// TODO use parameters to return the real value
	float a = 2.0f * M_PI * timeOfDay;
	return vec3(
		sinf(a),
		0.0f,
		-cosf(a));
}

void Sun::update(float dt) {
	if (dayLength > 0) {
		timeOfDay += dt / dayLength;
		while (timeOfDay >= 1.0f) {
			timeOfDay -= 1.0f;
		}
		direction = directionAtTime(timeOfDay);
	}
}
