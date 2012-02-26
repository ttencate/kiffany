#include "atmosphere.h"

#include "flags.h"

/* References:
 * Preetham, Shirley, Smits, "A practical model for daylight",
 *     http://www.cs.utah.edu/~shirley/papers/sunsky/sunsky.pdf
 * Nishita, Sirai, Tadamura, Nakamae, "Display of the Earth taking into account atmospheric scattering",
 *     http://citeseerx.ist.psu.edu/viewdoc/download?doi=10.1.1.145.5229&rep=rep1&type=pdf
 * Hoffman, Preetham, "Rendering outdoor light scattering in real time"
 *     http://developer.amd.com/media/gpu_assets/ATI-LightScattering.pdf
 * Bruneton, Neyret, "Precomputed atmospheric scattering",
 *     http://hal.archives-ouvertes.fr/docs/00/28/87/58/PDF/article.pdf
 * Preetham, "Modeling skylight and aerial perspective",
 *     http://developer.amd.com/media/gpu_assets/PreethamSig2003CourseNotes.pdf
 * http://http.developer.nvidia.com/GPUGems2/gpugems2_chapter16.html
 * http://www.springerlink.com/content/nrq497r40xmw4821/fulltext.pdf
 */

bool rayHitsHeight(Ray ray, float targetHeight) {
	return ray.height * sin(ray.angle) < targetHeight;
}

float rayLengthUpwards(Ray ray, float targetHeight) {
	float cosAngle = cos(ray.angle);
	return sqrt(
			pow2(ray.height) * (pow2(cosAngle) - 1.0f) +
			pow2(targetHeight))
		- ray.height * cosAngle;
}

float rayLengthToSameHeight(Ray ray) {
	return -2.0f * ray.height * cos(ray.angle);
}

float rayLengthDownwards(Ray ray, float targetHeight) {
	float cosAngle = cos(ray.angle);
	return -sqrt(
			pow2(ray.height) * (pow2(cosAngle) - 1.0f) +
			pow2(targetHeight))
		- ray.height * cosAngle;
}

float rayAngleUpwards(Ray ray, float targetHeight) {
	float rayAngle = asinf(clamp(ray.height * sinf(ray.angle) / targetHeight, -1.0f, 1.0f));
	return rayAngle;
}

float rayAngleToSameHeight(Ray ray) {
	return M_PI - ray.angle;
}

float rayAngleDownwards(Ray ray, float targetHeight) {
	float rayAngle = M_PI - asinf(clamp(ray.height * sinf(ray.angle) / targetHeight, -1.0f, 1.0f));
	return rayAngle;
}

Atmosphere::Atmosphere()
:
	lambda(680e-9f, 550e-9f, 440e-9f), // Bruneton and Neyret
	earthRadius(flags.earthRadius),
	atmosphereThickness(flags.atmosphereThickness),
	rayleighThickness(flags.rayleighThickness),
	mieThickness(flags.mieThickness),
	rayleighCoefficient(flags.rayleighCoefficient * vec3(1.00f, 2.33f, 5.71f)),
	mieCoefficient(flags.mieCoefficient),
	mieAbsorption(flags.mieAbsorption),
	mieDirectionality(flags.mieDirectionality)
{
}

AtmosphereLayers::Heights AtmosphereLayers::computeHeights(Atmosphere const &atmosphere) {
	Heights heights(numLayers);
	for (unsigned i = 0; i < numLayers - 1; ++i) {
		float const containedFraction = 1.0f - (float)i / (numLayers - 1);
		heights[i] = atmosphere.earthRadius - atmosphere.rayleighThickness * log(containedFraction);
	}
	heights[numLayers - 1] = std::max(heights[numLayers - 1] * 1.01f, atmosphere.earthRadius + atmosphere.atmosphereThickness);
	return heights;
}

AtmosphereLayers::Densities AtmosphereLayers::computeDensities(Atmosphere const &atmosphere, float thickness) {
	Densities densities(numLayers);
	for (unsigned i = 0; i < numLayers - 1; ++i) {
		// Density, averaged analytically over the space between the layers
		densities[i] = thickness / (heights[i + 1] - heights[i]) * (
				exp(-(heights[i] - atmosphere.earthRadius) / thickness) -
				exp(-(heights[i + 1] - atmosphere.earthRadius) / thickness));
	}
	densities[numLayers - 1] = 0.0f;
	return densities;
}

AtmosphereLayers::AtmosphereLayers(Atmosphere const &atmosphere, unsigned numLayers, unsigned numAngles)
:
	numLayers(numLayers),
	numAngles(numAngles),
	heights(computeHeights(atmosphere)),
	rayleighDensities(computeDensities(atmosphere, atmosphere.rayleighThickness)),
	mieDensities(computeDensities(atmosphere, atmosphere.mieThickness))
{
}

// Ray length from the given layer to the next,
// where 'next' might be the one above (for angle <= 0.5fpi),
// the layer itself (for 0.5fpi < angle < x),
// or the layer below it (for x <= angle).
inline float rayLengthToNextLayer(Ray ray, AtmosphereLayers const &layers, unsigned layer) {
	if (ray.angle <= 0.5f * M_PI && layer == layers.numLayers - 1) {
		// Ray goes up into space
		return 0.0f;
	} else if (ray.angle <= 0.5f * M_PI) {
		// Ray goes up, hits layer above
		return rayLengthUpwards(ray, layers.heights[layer + 1]);
	} else if (layer == 0) {
		// Ray goes down, into the ground; near infinite
		return 1e30f;
	} else if (!rayHitsHeight(ray, layers.heights[layer - 1])) {
		// Ray goes down, misses layer below, hits current layer from below
		return rayLengthToSameHeight(ray);
	} else {
		// Ray goes down, hits layer below
		return rayLengthDownwards(ray, layers.heights[layer - 1]);
	}
}

inline vec3 transmittanceToNextLayer(Ray ray, Atmosphere const &atmosphere, AtmosphereLayers const &layers, unsigned layer) {
	vec3 const rayleighExtinctionCoefficient = atmosphere.rayleighCoefficient;
	vec3 const mieExtinctionCoefficient = atmosphere.mieCoefficient + atmosphere.mieAbsorption;

	float rayLength = rayLengthToNextLayer(ray, layers, layer);
	unsigned nextLayer = ray.angle <= 0.5 * M_PI ? layer : (std::max(1u, layer) - 1);
	vec3 const extinction =
		rayleighExtinctionCoefficient * layers.rayleighDensities[nextLayer] +
		mieExtinctionCoefficient * layers.mieDensities[nextLayer];
	return exp(-extinction * rayLength);
}

void debugPrintTable(std::ostream &out, Vec3Table2D const &table) {
	unsigned const numLayers = table.getSize().x;
	unsigned const numAngles = table.getSize().y;
	out << std::fixed;
	out.precision(5);
	for (int layer = numLayers - 1; layer >= 0; --layer) {
		out << layer << "  ";
		for (unsigned a = 0; a < numAngles; ++a) {
			if (a == numAngles / 2) {
				out << "| ";
			}
			out << table.get(uvec2(layer, a)).r << ' ';
		}
		out << '\n';
	}
}

Vec3Table2D buildTransmittanceTable(Atmosphere const &atmosphere, AtmosphereLayers const &layers) {
	unsigned const numAngles = layers.numAngles;
	unsigned const numLayers = layers.numLayers;

	Vec3Table2D transmittanceTable = Vec3Table2D::createWithCoordsSizeAndOffset(
			uvec2(numLayers, numAngles),
			vec2(numLayers, M_PI * numAngles / (numAngles - 1)),
			vec2(0.0f, 0.0f));
	for (unsigned a = 0; a < numAngles; ++a) {
		float const angle = M_PI * a / (numAngles - 1);
		for (unsigned layer = 0; layer < numLayers; ++layer) {
			float const height = layers.heights[layer];
			Ray ray(height, angle);
			vec3 transmittance = transmittanceToNextLayer(ray, atmosphere, layers, layer);
			transmittanceTable.set(uvec2(layer, a), transmittance);
		}
	}

	//std::cout << "Transmittance table:\n";
	//debugPrintTable(std::cout, transmittanceTable);
	return transmittanceTable;
}

// Transmittance from the given layer to outer space,
// for the given angle on that layer.
// Zero if the earth is in between.
Vec3Table2D buildTotalTransmittanceTable(Atmosphere const &atmosphere, AtmosphereLayers const &layers, Vec3Table2D const &transmittanceTable) {
	unsigned const numAngles = layers.numAngles;
	unsigned const numLayers = layers.numLayers;

	Vec3Table2D totalTransmittanceTable = Vec3Table2D::createWithCoordsSizeAndOffset(
			uvec2(numLayers, numAngles),
			vec2(numLayers, M_PI * numAngles / (numAngles - 1)),
			vec2(0.0f, 0.0f));
	// For upward angles, cumulatively sum over all layers.
	for (int layer = numLayers - 1; layer >= 0; --layer) {
		for (unsigned a = 0; a < numAngles / 2; ++a) {
			float const angle = M_PI * a / (numAngles - 1);
			Ray ray(layers.heights[layer], angle);
			vec3 totalTransmittance;
			if (layer == (int)numLayers - 1) {
				// Ray goes directly into space
				totalTransmittance = vec3(1.0f);
			} else {
				// Ray passes through some layers
				float const nextAngle = rayAngleUpwards(ray, layers.heights[layer + 1]);
				
				totalTransmittance =
					transmittanceTable(vec2(layer, angle)) *
					totalTransmittanceTable(vec2(layer + 1, nextAngle));
			}
			totalTransmittanceTable.set(uvec2(layer, a), totalTransmittance);
		}
	}
	// We can only do downward angles after we've got all upward ones,
	// because (unless they hit the ground) they eventually start going up again.
	// Also, we need to do them in bottom-to-top order.
	for (unsigned layer = 0; layer < numLayers; ++layer) {
		for (unsigned a = numAngles / 2; a < numAngles; ++a) {
			float const angle = M_PI * a / (numAngles - 1);
			Ray ray(layers.heights[layer], angle);
			vec3 totalTransmittance;
			if (layer == 0) {
				// Ray goes directly into the ground
				totalTransmittance = vec3(0.0f);
			} else if (rayHitsHeight(ray, layers.heights[layer - 1])) {
				// Ray hits the layer below
				float const nextAngle = rayAngleDownwards(ray, layers.heights[layer - 1]);
				totalTransmittance =
					transmittanceTable(vec2(layer, angle)) *
					totalTransmittanceTable(vec2(layer - 1, nextAngle));
			} else {
				// Ray misses the layer below, hits the current one from below
				float const nextAngle = rayAngleToSameHeight(ray);
				totalTransmittance =
					transmittanceTable(vec2(layer, angle)) *
					totalTransmittanceTable(vec2(layer, nextAngle));
			}
			totalTransmittanceTable.set(uvec2(layer, a), totalTransmittance);
		}
	}

	//std::cout << "Total transmittance table:\n";
	//debugPrintTable(std::cout, totalTransmittanceTable);
	return totalTransmittanceTable;
}

