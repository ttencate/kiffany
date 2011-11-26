#ifndef SKY_H
#define SKY_H

#include "gl.h"
#include "maths.h"
#include "space.h"
#include "table.h"

#include <vector>

// TODO reorder arguments
// TODO measure from centre
bool rayHitsHeight(double startHeight, double targetHeight, double startAngle, double earthRadius);
double rayLengthBetweenHeights(double startHeight, double targetHeight, double startAngle, double earthRadius);
double rayAngleAtHeight(double startHeight, double targetHeight, double startAngle, double earthRadius);

class Scattering {

	protected:

		static dvec3 const lambda;

		Scattering(double height, dvec3 coefficient);

	public:

		double const height;
		dvec3 const coefficient;

		double densityAtHeight(double height) const;
		double opticalLengthBetweenHeights(double lowerHeight, double upperHeight, double lowerAngle, double earthRadius) const;

};

class RayleighScattering : public Scattering {
	dvec3 computeCoefficient() const;
	public:
		RayleighScattering();
		double phaseFunction(double lightAngle) const;
};

class MieScattering : public Scattering {
	dvec3 computeCoefficient() const;
	public:
		dvec3 const absorption;
		MieScattering();
		double phaseFunction(double lightAngle) const;
};

class Atmosphere {

	public:

		double const earthRadius;
		double const atmosphereHeight;

		RayleighScattering const rayleighScattering;
		MieScattering const mieScattering;

		Atmosphere(double earthRadius = 6371e3, double atmosphereHeight = 100e3);
};

class AtmosphereLayers {

	public:

		typedef std::vector<double> Heights;
	
	private:

		Heights computeHeights(double rayleighHeight, double atmosphereHeight);

	public:

		unsigned const numLayers;
		unsigned const numAngles;
		Heights const heights;

		AtmosphereLayers(Atmosphere const &atmosphere, unsigned numLayers);
};

typedef Table<dvec3, uvec2, dvec2> Dvec3Table2D;

Dvec3Table2D buildTransmittanceTable(Atmosphere const &atmosphere, AtmosphereLayers const &layers);
Dvec3Table2D buildSunTransmittanceTable(Atmosphere const &atmosphere, AtmosphereLayers const &layers);

class Scatterer {

	Atmosphere atmosphere;
	AtmosphereLayers layers;

	Dvec3Table2D transmittanceTable;
	Dvec3Table2D sunTransmittanceTable;

	public:

		Scatterer(Atmosphere const &atmosphere, AtmosphereLayers const &layers);

		dvec3 scatteredLight(dvec3 direction, dvec3 sunDirection, dvec3 sunColor) const;

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
