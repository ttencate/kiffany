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

		GLAtmosphere const &getAtmosphere() const { return *atmosphere; }
		Sun const &getSun() const { return *sun; }

		vec4 ambientColor() const;

};

#endif
