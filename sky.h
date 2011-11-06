#ifndef SKY_H
#define SKY_H

#include "gl.h"

#include <boost/scoped_array.hpp>

class Sky {

	unsigned const size;

	GLBuffer vertices;
	GLTexture texture;

	boost::scoped_array<unsigned char> textureImage;

	void generateFace(GLenum face);
	void generateFaces();

	public:

		Sky();

		void update(float dt);
		void render();

};

#endif
