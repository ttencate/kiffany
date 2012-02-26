#ifndef LIGHTING_H
#define LIGHTING_H

#include "maths.h"

class GLAtmosphere;
class Sun;

class Lighting {

	GLAtmosphere const *atmosphere;
	Sun const *sun;

	public:

		Lighting(GLAtmosphere const *atmosphere, Sun const *sun);

		vec4 ambientColor() const;
		vec4 sunColor() const;
		vec3 sunDirection() const;

};

#endif
