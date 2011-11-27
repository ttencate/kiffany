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

// TODO see if it still works if we replace double by float

struct Ray {
	double height;
	double angle;
	Ray(double height, double angle) : height(height), angle(angle) {}
};

bool rayHitsHeight(Ray ray, double targetHeight);

double rayLengthUpwards(Ray ray, double targetHeight);
double rayLengthToSameHeight(Ray ray);
double rayLengthDownwards(Ray ray, double targetHeight);

double rayAngleUpwards(Ray ray, double targetHeight);
double rayAngleToSameHeight(Ray ray);
double rayAngleDownwards(Ray ray, double targetHeight);

class Scattering {

	protected:

		static dvec3 const lambda;

		Scattering(double unityHeight, double thickness, dvec3 coefficient);

	public:

		double const unityHeight;
		double const thickness;
		dvec3 const coefficient;

		double densityAtHeight(double height) const;

};

class RayleighScattering : public Scattering {
	dvec3 computeCoefficient() const;
	public:
		RayleighScattering(double unityHeight);
		double phaseFunction(double lightAngle) const;
};

class MieScattering : public Scattering {
	dvec3 computeCoefficient() const;
	public:
		dvec3 const absorption;
		MieScattering(double unityHeight);
		double phaseFunction(double lightAngle) const;
};

class Atmosphere {

	public:

		double const earthRadius;
		double const thickness;

		RayleighScattering const rayleighScattering;
		MieScattering const mieScattering;

		Atmosphere(double earthRadius = 6371e3, double thickness = 100e3);
};

class AtmosphereLayers {

	public:

		typedef std::vector<double> Heights;
	
	private:

		Heights computeHeights(double earthRadius, double rayleighHeight, double atmosphereHeight);

	public:

		unsigned const numLayers;
		unsigned const numAngles;
		Heights const heights;

		AtmosphereLayers(Atmosphere const &atmosphere, unsigned numLayers);
};

typedef Table<dvec3, uvec2, dvec2> Dvec3Table2D;

double rayLengthToNextLayer(Ray ray, AtmosphereLayers const &layers, unsigned layer);
dvec3 transmittanceToNextLayer(Ray ray, Atmosphere const &atmosphere, AtmosphereLayers const &layers, unsigned layer);

Dvec3Table2D buildTransmittanceTable(Atmosphere const &atmosphere, AtmosphereLayers const &layers);
Dvec3Table2D buildTotalTransmittanceTable(Atmosphere const &atmosphere, AtmosphereLayers const &layers, Dvec3Table2D const &transmittanceTable);

// TODO in the shader world, everything below this line needs refactor

class Sky : boost::noncopyable {

	Atmosphere atmosphere;
	AtmosphereLayers layers;
	Sun const *sun;

	Dvec3Table2D transmittanceTable;
	Dvec3Table2D totalTransmittanceTable;
	GLTexture transmittanceTexture;
	GLTexture totalTransmittanceTexture;

	unsigned const size;
	boost::scoped_array<unsigned char> textureImage;

	Buffer vertices;
	GLTexture texture;

	ShaderProgram shaderProgram;

	dvec3 computeColor(dvec3 viewDirection);
	void generateFace(GLenum face, dvec3 base, dvec3 xBasis, dvec3 yBasis);
	void generateFaces();

	public:

		Sky(Atmosphere const &atmosphere, AtmosphereLayers const &layers, Sun const *sun);

		void setSun(Sun const *sun) { this->sun = sun; }

		void update(float dt);
		void render();

};

#endif
