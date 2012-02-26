#include "sky.h"

#include <boost/assert.hpp>

#include <algorithm>
#include <limits>

Sky::Sky(GLAtmosphere const *atmosphere, Sun const *sun)
:
	atmosphere(atmosphere),
	sun(sun)
{

	int const v[] = {
		-1, -1, -1,
		-1,  1, -1,
		-1,  1,  1,
		-1, -1,  1,

		 1, -1, -1,
		 1, -1,  1,
		 1,  1,  1,
		 1,  1, -1,

		-1, -1, -1,
		-1, -1,  1,
		 1, -1,  1,
		 1, -1, -1,

		-1,  1, -1,
		 1,  1, -1,
		 1,  1,  1,
		-1,  1,  1,

		-1, -1, -1,
		 1, -1, -1,
		 1,  1, -1,
		-1,  1, -1,
		
		-1, -1,  1,
		-1,  1,  1,
		 1,  1,  1,
		 1, -1,  1
	};
	vertices.putData(sizeof(v), v, GL_STATIC_DRAW);

	shaderProgram.loadAndLink("shaders/sky.vert", "shaders/sky.frag");
}

void Sky::update(float dt) {
}

void Sky::render() {
	glDisable(GL_DEPTH_TEST);
	glDepthMask(GL_FALSE);

	glEnableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);	

	bindBuffer(GL_ARRAY_BUFFER, vertices);
	glVertexPointer(3, GL_INT, 0, 0);

	useProgram(shaderProgram.getProgram());

	AtmosParams const &params = atmosphere->getParams();
	AtmosLayers const &layers = atmosphere->getLayers();

	shaderProgram.setUniform("params.earthRadius", params.earthRadius);
	shaderProgram.setUniform("params.rayleighCoefficient", vec3(params.rayleighCoefficient));
	shaderProgram.setUniform("params.mieCoefficient", vec3(params.mieCoefficient));
	shaderProgram.setUniform("params.mieDirectionality", params.mieDirectionality);
	shaderProgram.setUniform("params.numLayers", (int)params.numLayers);
	shaderProgram.setUniform("params.numAngles", (int)params.numAngles);
	shaderProgram.setUniform("sun.angularRadius", sun->getAngularRadius());
	shaderProgram.setUniform("sun.color", sun->getColor());
	shaderProgram.setUniform("sun.direction", sun->getDirection());
	shaderProgram.setUniform("layers.heights", layers.heights);
	shaderProgram.setUniform("layers.rayleighDensities", layers.rayleighDensities);
	shaderProgram.setUniform("layers.mieDensities", layers.mieDensities);

	activeTexture(0);
	bindTexture(GL_TEXTURE_RECTANGLE, atmosphere->getTransmittanceTexture());
	shaderProgram.setUniform("transmittanceSampler", 0);

	activeTexture(1);
	bindTexture(GL_TEXTURE_RECTANGLE, atmosphere->getTotalTransmittanceTexture());
	shaderProgram.setUniform("totalTransmittanceSampler", 1);

	bindFragDataLocation(shaderProgram.getProgram(), 0, "scatteredLight");

	glDrawArrays(GL_QUADS, 0, vertices.getSizeInBytes() / sizeof(int) / 3);

	useFixedProcessing();

	glDepthMask(GL_TRUE);
	glEnable(GL_DEPTH_TEST);
}
