#ifndef SKY_H
#define SKY_H

#include "gl.h"
#include "maths.h"

#include <boost/scoped_array.hpp>

class Sky {

	unsigned const size;

	GLBuffer vertices;
	GLTexture texture;

	boost::scoped_array<unsigned char> textureImage;

	vec3 computeColor(vec3 direction);
	void generateFace(GLenum face, vec3 base, vec3 xBasis, vec3 yBasis);
	void generateFaces();

	public:

		Sky();

		void update(float dt);
		void render();

};

#endif
