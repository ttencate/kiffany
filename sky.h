#ifndef SKY_H
#define SKY_H

#include "buffer.h"
#include "gl.h"
#include "maths.h"
#include "shader.h"
#include "space.h"
#include "table.h"

#include <vector>

#include <boost/noncopyable.hpp>

/*
 * Conventions:
 * - all angles are in radians
 * - all distances are in metres
 * - all angles are zenith angles (0 <= a <= pi)
 *   with 0 being straight up (away from the centre of the earth)
 *   and pi being straight down (towards the centre of the earth)
 * - all heights are measured from the centre of the earth
 * - things measured from sea level are called "thickness"
 */

// TODO elevations instead of zenith angles?
// TODO remove sqr, use pow2

struct Ray {
	float height;
	float angle;
	Ray(float height, float angle) : height(height), angle(angle) {}
};

bool rayHitsHeight(Ray ray, float targetHeight);

float rayLengthUpwards(Ray ray, float targetHeight);
float rayLengthToSameHeight(Ray ray);
float rayLengthDownwards(Ray ray, float targetHeight);

float rayAngleUpwards(Ray ray, float targetHeight);
float rayAngleToSameHeight(Ray ray);
float rayAngleDownwards(Ray ray, float targetHeight);

class Scattering {

	protected:

		static vec3 const lambda;

		Scattering(float unityHeight, float thickness, vec3 coefficient);

	public:

		float const unityHeight;
		float const thickness;
		vec3 const coefficient;

		float densityAtHeight(float height) const;

};

class RayleighScattering : public Scattering {
	vec3 computeCoefficient() const;
	public:
		RayleighScattering(float unityHeight);
		float phaseFunction(float lightAngle) const;
};

class MieScattering : public Scattering {
	vec3 computeCoefficient() const;
	public:
		vec3 const absorption;
		MieScattering(float unityHeight);
		float phaseFunction(float lightAngle) const;
};

class Atmosphere {

	public:

		float const earthRadius;
		float const thickness;

		RayleighScattering const rayleighScattering;
		MieScattering const mieScattering;

		Atmosphere(float earthRadius = 6371e3, float thickness = 100e3);
};

class AtmosphereLayers {

	public:

		typedef std::vector<float> Heights;
	
	private:

		Heights computeHeights(float earthRadius, float rayleighHeight, float atmosphereHeight);

	public:

		unsigned const numLayers;
		unsigned const numAngles;
		Heights const heights;

		AtmosphereLayers(Atmosphere const &atmosphere, unsigned numLayers);
};

typedef Table<vec3, uvec2, vec2> Vec3Table2D;

float rayLengthToNextLayer(Ray ray, AtmosphereLayers const &layers, unsigned layer);
vec3 transmittanceToNextLayer(Ray ray, Atmosphere const &atmosphere, AtmosphereLayers const &layers, unsigned layer);

Vec3Table2D buildTransmittanceTable(Atmosphere const &atmosphere, AtmosphereLayers const &layers);
Vec3Table2D buildTotalTransmittanceTable(Atmosphere const &atmosphere, AtmosphereLayers const &layers, Vec3Table2D const &transmittanceTable);

// TODO in the shader world, everything below this line needs refactor

class Sky : boost::noncopyable {

	Atmosphere atmosphere;
	AtmosphereLayers layers;
	Sun const *sun;

	Vec3Table2D transmittanceTable;
	Vec3Table2D totalTransmittanceTable;
	GLTexture transmittanceTexture;
	GLTexture totalTransmittanceTexture;

	unsigned const size;
	boost::scoped_array<unsigned char> textureImage;

	Buffer vertices;
	GLTexture texture;

	ShaderProgram shaderProgram;

	vec3 computeColor(vec3 viewDirection);
	void generateFace(GLenum face, vec3 base, vec3 xBasis, vec3 yBasis);
	void generateFaces();

	public:

		Sky(Atmosphere const &atmosphere, AtmosphereLayers const &layers, Sun const *sun);

		void setSun(Sun const *sun) { this->sun = sun; }

		void update(float dt);
		void render();

};

#endif
