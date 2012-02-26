#include "sky.h"

#include <boost/assert.hpp>
#include <boost/static_assert.hpp>

#include <algorithm>
#include <limits>

void tableToTexture(Vec3Table2D const &table, GLTexture &texture) {
	BOOST_STATIC_ASSERT(sizeof(vec3) == 3 * sizeof(float));
	bindTexture(GL_TEXTURE_RECTANGLE, texture);
	glTexImage2D(GL_TEXTURE_RECTANGLE, 0, GL_RGBA32F, table.getSize().x, table.getSize().y, 0, GL_RGB, GL_FLOAT, (float const *)table.raw());
	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

Sky::Sky(Atmosphere const &atmosphere, AtmosphereLayers const &layers, Sun const *sun)
:
	atmosphere(atmosphere),
	layers(layers),
	sun(sun),
	transmittanceTable(buildTransmittanceTable(atmosphere, layers)),
	totalTransmittanceTable(buildTotalTransmittanceTable(atmosphere, layers, transmittanceTable))
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

	tableToTexture(transmittanceTable, transmittanceTexture);
	tableToTexture(totalTransmittanceTable, totalTransmittanceTexture);

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

	shaderProgram.setUniform("atmosphere.earthRadius", atmosphere.earthRadius);
	shaderProgram.setUniform("atmosphere.rayleighCoefficient", vec3(atmosphere.rayleighCoefficient));
	shaderProgram.setUniform("atmosphere.mieCoefficient", vec3(atmosphere.mieCoefficient));
	shaderProgram.setUniform("atmosphere.mieDirectionality", atmosphere.mieDirectionality);
	shaderProgram.setUniform("sun.angularRadius", sun->getAngularRadius());
	shaderProgram.setUniform("sun.color", sun->getColor());
	shaderProgram.setUniform("sun.direction", sun->getDirection());
	shaderProgram.setUniform("layers.numLayers", (int)layers.numLayers);
	shaderProgram.setUniform("layers.numAngles", (int)layers.numAngles);
	shaderProgram.setUniform("layers.heights", layers.heights);
	shaderProgram.setUniform("layers.rayleighDensities", layers.rayleighDensities);
	shaderProgram.setUniform("layers.mieDensities", layers.mieDensities);

	activeTexture(1);
	bindTexture(GL_TEXTURE_RECTANGLE, totalTransmittanceTexture);
	activeTexture(0);
	bindTexture(GL_TEXTURE_RECTANGLE, transmittanceTexture);
	shaderProgram.setUniform("transmittanceSampler", 0);
	shaderProgram.setUniform("totalTransmittanceSampler", 1);

	bindFragDataLocation(shaderProgram.getProgram(), 0, "scatteredLight");

	glDrawArrays(GL_QUADS, 0, vertices.getSizeInBytes() / sizeof(int) / 3);

	useFixedProcessing();

	glDepthMask(GL_TRUE);
	glEnable(GL_DEPTH_TEST);
}
