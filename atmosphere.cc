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

bool Ray::hitsHeight(float targetHeight) const {
	return height * sin(angle) < targetHeight;
}

float Ray::lengthUpwards(float targetHeight) const {
	float cosAngle = cos(angle);
	return sqrt(
			pow2(height) * (pow2(cosAngle) - 1.0f) +
			pow2(targetHeight))
		- height * cosAngle;
}

float Ray::lengthToSameHeight() const {
	return -2.0f * height * cos(angle);
}

float Ray::lengthDownwards(float targetHeight) const {
	float cosAngle = cos(angle);
	return -sqrt(
			pow2(height) * (pow2(cosAngle) - 1.0f) +
			pow2(targetHeight))
		- height * cosAngle;
}

float Ray::angleUpwards(float targetHeight) const {
	return asinf(clamp(height * sinf(angle) / targetHeight, -1.0f, 1.0f));
}

float Ray::angleToSameHeight() const {
	return M_PI - angle;
}

float Ray::angleDownwards(float targetHeight) const {
	return M_PI - asinf(clamp(height * sinf(angle) / targetHeight, -1.0f, 1.0f));
}

AtmosParams::AtmosParams()
:
	lambda(680e-9f, 550e-9f, 440e-9f), // Bruneton and Neyret
	earthRadius(flags.earthRadius),
	atmosphereThickness(flags.atmosphereThickness),
	rayleighThickness(flags.rayleighThickness),
	mieThickness(flags.mieThickness),
	rayleighCoefficient(flags.rayleighCoefficient * vec3(1.00f, 2.33f, 5.71f)),
	mieCoefficient(flags.mieCoefficient),
	mieAbsorption(flags.mieAbsorption),
	mieDirectionality(flags.mieDirectionality),
	numLayers(flags.atmosphereLayers),
	numAngles(flags.atmosphereAngles)
{
}

AtmosLayers::AtmosLayers(AtmosParams const &params)
:
	heights(computeHeights(params)),
	rayleighDensities(computeDensities(params, params.rayleighThickness)),
	mieDensities(computeDensities(params, params.mieThickness))
{
}

// Ray length from the given layer to the next,
// where 'next' might be the one above (for angle <= 0.5pi),
// the layer itself (for 0.5pi < angle < x),
// or the layer below it (for x <= angle).
float AtmosLayers::rayLengthToNextLayer(Ray ray, unsigned layer) const {
	if (ray.angle <= 0.5f * M_PI && layer == heights.size() - 1) {
		// Ray goes up into space
		return 0.0f;
	} else if (ray.angle <= 0.5f * M_PI) {
		// Ray goes up, hits layer above
		return ray.lengthUpwards(heights[layer + 1]);
	} else if (layer == 0) {
		// Ray goes down, into the ground; near infinite
		return 1e30f;
	} else if (!ray.hitsHeight(heights[layer - 1])) {
		// Ray goes down, misses layer below, hits current layer from below
		return ray.lengthToSameHeight();
	} else {
		// Ray goes down, hits layer below
		return ray.lengthDownwards(heights[layer - 1]);
	}
}

AtmosLayers::Heights AtmosLayers::computeHeights(AtmosParams const &params) {
	unsigned const numLayers = params.numLayers;
	Heights heights(numLayers);
	for (unsigned i = 0; i < numLayers - 1; ++i) {
		float const containedFraction = 1.0f - (float)i / (numLayers - 1);
		heights[i] = params.earthRadius - params.rayleighThickness * log(containedFraction);
	}
	heights[numLayers - 1] = std::max(heights[numLayers - 1] * 1.01f, params.earthRadius + params.atmosphereThickness);
	return heights;
}

AtmosLayers::Densities AtmosLayers::computeDensities(AtmosParams const &params, float thickness) {
	unsigned const numLayers = params.numLayers;
	Densities densities(numLayers);
	for (unsigned i = 0; i < numLayers - 1; ++i) {
		// Density, averaged analytically over the space between the layers
		densities[i] = thickness / (heights[i + 1] - heights[i]) * (
				exp(-(heights[i] - params.earthRadius) / thickness) -
				exp(-(heights[i + 1] - params.earthRadius) / thickness));
	}
	densities[numLayers - 1] = 0.0f;
	return densities;
}

Atmosphere::Atmosphere(AtmosParams const &params)
:
	params(params),
	layers(params),
	transmittanceTable(buildTransmittanceTable()),
	totalTransmittanceTable(buildTotalTransmittanceTable())
{
}

inline vec3 Atmosphere::transmittanceToNextLayer(Ray ray, unsigned layer) const {
	vec3 const rayleighExtinctionCoefficient = params.rayleighCoefficient;
	vec3 const mieExtinctionCoefficient = params.mieCoefficient + params.mieAbsorption;

	float rayLength = layers.rayLengthToNextLayer(ray, layer);
	unsigned nextLayer = ray.angle <= 0.5f * M_PI ? layer : (std::max(1u, layer) - 1);
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

Vec3Table2D Atmosphere::buildTransmittanceTable() const {
	unsigned const numAngles = params.numAngles;
	unsigned const numLayers = params.numLayers;

	Vec3Table2D transmittanceTable = Vec3Table2D::createWithCoordsSizeAndOffset(
			uvec2(numLayers, numAngles),
			vec2(numLayers, M_PI * numAngles / (numAngles - 1)),
			vec2(0.0f, 0.0f));
	for (unsigned a = 0; a < numAngles; ++a) {
		float const angle = M_PI * a / (numAngles - 1);
		for (unsigned layer = 0; layer < numLayers; ++layer) {
			float const height = layers.heights[layer];
			Ray ray(height, angle);
			vec3 transmittance = transmittanceToNextLayer(ray, layer);
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
Vec3Table2D Atmosphere::buildTotalTransmittanceTable() const {
	unsigned const numAngles = params.numAngles;
	unsigned const numLayers = params.numLayers;

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
				float const nextAngle = ray.angleUpwards(layers.heights[layer + 1]);
				
				totalTransmittance =
					transmittanceToNextLayer(ray, layer) *
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
			} else if (ray.hitsHeight(layers.heights[layer - 1])) {
				// Ray hits the layer below
				float const nextAngle = ray.angleDownwards(layers.heights[layer - 1]);
				totalTransmittance =
					transmittanceToNextLayer(ray, layer) *
					totalTransmittanceTable(vec2(layer - 1, nextAngle));
			} else {
				// Ray misses the layer below, hits the current one from below
				float const nextAngle = ray.angleToSameHeight();
				totalTransmittance =
					transmittanceToNextLayer(ray, layer) *
					totalTransmittanceTable(vec2(layer, nextAngle));
			}
			totalTransmittanceTable.set(uvec2(layer, a), totalTransmittance);
		}
	}

	//std::cout << "Total transmittance table:\n";
	//debugPrintTable(std::cout, totalTransmittanceTable);
	return totalTransmittanceTable;
}

