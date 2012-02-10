#include "lighting.h"

#include "gl.h"

Lighting::Lighting(Sun const *sun)
:
	sun(sun)
{
}

vec4 Lighting::ambientColor() const {
	float const af = sunDirection().z;
	return vec4(af * 0.45f, af * 0.5f, af * 0.55f, 1.0f);
}

vec4 Lighting::sunColor() const {
	float const df = smoothstep(0.0f, 0.3f, sunDirection().z);
	return vec4(df * 1.0f, df * 0.95f, df * 0.9f, 1.0f);
}

vec3 Lighting::sunDirection() const {
	return sun->getDirection();
}
