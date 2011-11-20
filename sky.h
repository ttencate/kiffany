#ifndef SKY_H
#define SKY_H

#include "gl.h"
#include "maths.h"
#include "table.h"

#include <vector>

typedef std::vector<double> LayerHeights;

LayerHeights computeLayerHeights(unsigned numLayers, double rayleighHeight, double atmosphereHeight);

class Atmosphere {

	dvec3 rayleighScatteringCoefficient;
	dvec3 mieScatteringCoefficient;

	double earthRadius;
	double rayleighHeight;
	double mieHeight;
	double atmosphereHeight;
	unsigned numAngles;
	LayerHeights layerHeights;

	void updateLayerHeights(unsigned numLayers);

	public:

		Atmosphere();

		dvec3 getRayleighScatteringCoefficient() const { return rayleighScatteringCoefficient; }
		dvec3 getMieScatteringCoefficient() const { return mieScatteringCoefficient; }
		double getEarthRadius() const { return earthRadius; }
		double getRayleighHeight() const { return rayleighHeight; }
		double getMieHeight() const { return mieHeight; }

		unsigned getNumAngles() const { return numAngles; }
		unsigned getNumLayers() const { return layerHeights.size(); }
		LayerHeights const &getLayerHeights() const { return layerHeights; }
		double getLayerHeight(unsigned layer) const { return layerHeights[layer]; }

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

typedef Table<dvec3, uvec2, dvec2> Dvec3Table2D;

Dvec3Table2D buildOpticalLengthTable(Atmosphere const &atmosphere);
Dvec3Table2D buildOpticalDepthTable(Atmosphere const &atmosphere);
Dvec3Table2D buildSunAttenuationTable(Atmosphere const &atmosphere, Dvec3Table2D const &opticalLengthTable);

class Scatterer {

	Atmosphere atmosphere;

	Dvec3Table2D opticalLengthTable;
	Dvec3Table2D sunAttenuationTable;

	public:

		Scatterer(Atmosphere const &atmosphere);

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

		Sky(Atmosphere const &atmosphere);

		void update(float dt);
		void render();

};

#endif
