#include "lighting.h"

#include "atmosphere.h"
#include "gl.h"
#include "space.h"

Lighting::Lighting(GLAtmosphere const *atmosphere, Sun const *sun)
:
	atmosphere(atmosphere),
	sun(sun)
{
}

vec4 Lighting::ambientColor() const {
	float const af = sun->getDirection().z;
	return vec4(af * 0.45f, af * 0.5f, af * 0.55f, 1.0f);
}
