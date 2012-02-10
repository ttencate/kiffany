#ifndef LIGHTING_H
#define LIGHTING_H

#include "space.h"

class Lighting {

	Sun const *sun;

	public:

		Lighting(Sun const *sun);

		vec4 ambientColor() const;
		vec4 sunColor() const;
		vec3 sunDirection() const;

};

#endif
