#ifndef ATMOSPHERE_H
#define ATMOSPHERE_H

#include "maths.h"
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

struct Atmosphere {

	vec3 lambda;

	float earthRadius;
	float atmosphereThickness;
	float rayleighThickness;
	float mieThickness;

	vec3 rayleighCoefficient;
	vec3 mieCoefficient;
	vec3 mieAbsorption;
	float mieDirectionality;

	Atmosphere();
};

class AtmosphereLayers {

	public:

		typedef std::vector<float> Heights;
		typedef std::vector<float> Densities;
	
	private:

		Heights computeHeights(Atmosphere const &atmosphere);
		Densities computeDensities(Atmosphere const &atmosphere, float thickness);

	public:

		unsigned const numLayers;
		unsigned const numAngles;
		Heights const heights;
		Densities const rayleighDensities;
		Densities const mieDensities;

		AtmosphereLayers(Atmosphere const &atmosphere, unsigned numLayers, unsigned numAngles);
};

typedef Table<vec3, uvec2, vec2> Vec3Table2D;

float rayLengthToNextLayer(Ray ray, AtmosphereLayers const &layers, unsigned layer);
vec3 transmittanceToNextLayer(Ray ray, Atmosphere const &atmosphere, AtmosphereLayers const &layers, unsigned layer);

Vec3Table2D buildTransmittanceTable(Atmosphere const &atmosphere, AtmosphereLayers const &layers);
Vec3Table2D buildTotalTransmittanceTable(Atmosphere const &atmosphere, AtmosphereLayers const &layers, Vec3Table2D const &transmittanceTable);

#endif
