#ifndef LIGHTING_H
#define LIGHTING_H

#include "space.h"

class Lighting {

	Sun const *sun;

	public:

		Lighting(Sun const *sun);

		void setup();

};

#endif
