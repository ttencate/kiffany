#ifndef SKY_H
#define SKY_H

#include "gl.h"
#include "maths.h"
#include "space.h"
#include "table.h"

#include <vector>

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

class Scatterer {

	Atmosphere atmosphere;
	AtmosphereLayers layers;

	Dvec3Table2D transmittanceTable;
	Dvec3Table2D totalTransmittanceTable;

	public:

		Scatterer(Atmosphere const &atmosphere, AtmosphereLayers const &layers);

		dvec3 scatteredLight(dvec3 viewDirection, Sun const &sun) const;

};

class Sky {

	Scatterer const scatterer;
	Sun const *sun;

	unsigned const size;
	boost::scoped_array<unsigned char> textureImage;

	GLBuffer vertices;
	GLTexture texture;

	dvec3 computeColor(vec3 direction);
	void generateFace(GLenum face, vec3 base, vec3 xBasis, vec3 yBasis);
	void generateFaces();

	public:

		Sky(Scatterer const &scatterer, Sun const *sun);

		void setSun(Sun const *sun) { this->sun = sun; }

		void update(float dt);
		void render();

};

#endif
