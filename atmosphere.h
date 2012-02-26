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

	bool hitsHeight(float targetHeight) const;

	float lengthUpwards(float targetHeight) const;
	float lengthToSameHeight() const;
	float lengthDownwards(float targetHeight) const;

	float angleUpwards(float targetHeight) const;
	float angleToSameHeight() const;
	float angleDownwards(float targetHeight) const;

};

struct AtmosParams {

	vec3 lambda;

	float earthRadius;
	float atmosphereThickness;
	float rayleighThickness;
	float mieThickness;

	vec3 rayleighCoefficient;
	vec3 mieCoefficient;
	vec3 mieAbsorption;
	float mieDirectionality;

	unsigned numLayers;
	unsigned numAngles;

	AtmosParams();
};

class AtmosLayers {

	public:

		typedef std::vector<float> Heights;
		typedef std::vector<float> Densities;

		Heights const heights;
		Densities const rayleighDensities;
		Densities const mieDensities;

		AtmosLayers(AtmosParams const &atmosphere);

		float rayLengthToNextLayer(Ray ray, unsigned layer) const;
	
	private:

		Heights computeHeights(AtmosParams const &atmosphere);
		Densities computeDensities(AtmosParams const &atmosphere, float thickness);
};

typedef Table<vec3, uvec2, vec2> Vec3Table2D;

vec3 transmittanceToNextLayer(Ray ray, AtmosParams const &params, AtmosLayers const &layers, unsigned layer);

Vec3Table2D buildTransmittanceTable(AtmosParams const &params, AtmosLayers const &layers);
Vec3Table2D buildTotalTransmittanceTable(AtmosParams const &params, AtmosLayers const &layers);

#endif
