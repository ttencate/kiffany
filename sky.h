#ifndef SKY_H
#define SKY_H

#include "atmosphere.h"
#include "buffer.h"
#include "gl.h"
#include "shader.h"
#include "space.h"

#include <vector>

#include <boost/noncopyable.hpp>

// TODO in the shader world, everything below this line needs refactor

class Sky : boost::noncopyable {

	Atmosphere const atmosphere;
	AtmosphereLayers const layers;
	Sun const *sun;

	Vec3Table2D transmittanceTable;
	Vec3Table2D totalTransmittanceTable;

	GLTexture transmittanceTexture;
	GLTexture totalTransmittanceTexture;

	Buffer vertices;

	ShaderProgram shaderProgram;

	public:

		Sky(Atmosphere const &atmosphere, AtmosphereLayers const &layers, Sun const *sun);

		void setSun(Sun const *sun) { this->sun = sun; }

		void update(float dt);
		void render();

};

#endif
