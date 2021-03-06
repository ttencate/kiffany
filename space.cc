#include "space.h"

#include "flags.h"

Sun::Sun(float season, float latitude, float axialTilt, float dayLength, float angularRadius, float brightness, vec3 color, float timeOfDay)
:
	season(season),
	latitude(latitude),
	axialTilt(axialTilt),
	dayLength(dayLength),
	angularRadius(angularRadius),
	brightness(brightness),
	color(color),
	timeOfDay(timeOfDay)
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
		if (flags.skipNight && timeOfDay > 0.77f) {
			timeOfDay += 0.46f;
		}
		while (timeOfDay >= 1.0f) {
			timeOfDay -= 1.0f;
		}
		direction = directionAtTime(timeOfDay);
	}
}
