#ifndef SKY_H
#define SKY_H

#include "atmosphere.h"
#include "buffer.h"
#include "shader.h"
#include "space.h"

#include <vector>

#include <boost/noncopyable.hpp>

class Sky : boost::noncopyable {

	GLAtmosphere const *atmosphere;
	Sun const *sun;

	Buffer vertices;

	ShaderProgram shaderProgram;

	public:

		Sky(GLAtmosphere const *atmosphere, Sun const *sun);

		void setSun(Sun const *sun) { this->sun = sun; }

		void update(float dt);
		void render();

};

#endif
