#ifndef SKY_H
#define SKY_H

#include "gl.h"
#include "maths.h"
#include "table.h"

#include <vector>

class Atmosphere {

	static dvec3 const lambda;

	dvec3 computeRayleighScatteringCoefficient() const;
	dvec3 computeMieScatteringCoefficient() const;

	public:

		double const earthRadius;
		double const rayleighHeight;
		double const mieHeight;
		double const atmosphereHeight;

		dvec3 const rayleighScatteringCoefficient;
		dvec3 const mieScatteringCoefficient;

		Atmosphere();

		double rayleighDensityAtHeight(double height) const;
		double mieDensityAtHeight(double height) const;

		double rayAngleAtHeight(double angle, double height) const;
		double rayLengthBetweenHeights(double angle, double lowerHeight, double upperHeight) const;
		double rayLengthToHeight(double angle, double height) const;

		double rayleighPhaseFunction(double angle) const;
		double miePhaseFunction(double angle) const;

		dvec3 rayleighScatteringAtHeight(double angle, double height) const;
		dvec3 mieScatteringAtHeight(double angle, double height) const;

		dvec3 attenuationFromOpticalLength(dvec3 opticalLength) const;

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

		AtmosphereLayers(Atmosphere const &atmosphere);
};

typedef Table<dvec3, uvec2, dvec2> Dvec3Table2D;

Dvec3Table2D buildOpticalLengthTable(Atmosphere const &atmosphere, AtmosphereLayers const &layers);
Dvec3Table2D buildOpticalDepthTable(Atmosphere const &atmosphere, AtmosphereLayers const &layers);
Dvec3Table2D buildSunAttenuationTable(Atmosphere const &atmosphere, AtmosphereLayers const &layers, Dvec3Table2D const &opticalLengthTable);

class Scatterer {

	Atmosphere atmosphere;
	AtmosphereLayers layers;

	Dvec3Table2D opticalLengthTable;
	Dvec3Table2D sunAttenuationTable;

	public:

		Scatterer(Atmosphere const &atmosphere, AtmosphereLayers const &layers);

		dvec3 scatteredLightFactor(dvec3 direction, dvec3 sunDirection) const;

};

class Sky {

	Scatterer const scatterer;

	unsigned const size;
	boost::scoped_array<unsigned char> textureImage;

	GLBuffer vertices;
	GLTexture texture;

	dvec3 computeColor(vec3 direction);
	void generateFace(GLenum face, vec3 base, vec3 xBasis, vec3 yBasis);
	void generateFaces();

	public:

		Sky(Scatterer const &scatterer);

		void update(float dt);
		void render();

};

#endif
