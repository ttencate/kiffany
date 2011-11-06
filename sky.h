#ifndef SKY_H
#define SKY_H

#include "gl.h"

class Sky {

	GLBuffer vertices;
	GLTexture texture;

	public:

		Sky();

		void update(float dt);
		void render();

};

#endif
