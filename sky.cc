#include "sky.h"

#include "shader.h"

#include <boost/assert.hpp>
#include <boost/static_assert.hpp>

#include <algorithm>
#include <limits>

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

inline bool rayHitsHeight(Ray ray, float targetHeight) {
	return ray.height * sin(ray.angle) < targetHeight;
}

inline float rayLengthUpwards(Ray ray, float targetHeight) {
	float cosAngle = cos(ray.angle);
	return sqrt(
			pow2(ray.height) * (pow2(cosAngle) - 1.0f) +
			pow2(targetHeight))
		- ray.height * cosAngle;
}

inline float rayLengthToSameHeight(Ray ray) {
	return -2.0f * ray.height * cos(ray.angle);
}

inline float rayLengthDownwards(Ray ray, float targetHeight) {
	float cosAngle = cos(ray.angle);
	return -sqrt(
			pow2(ray.height) * (pow2(cosAngle) - 1.0f) +
			pow2(targetHeight))
		- ray.height * cosAngle;
}

inline float rayAngleUpwards(Ray ray, float targetHeight) {
	float rayAngle = asin(ray.height * sin(ray.angle) / targetHeight);
	return rayAngle;
}

inline float rayAngleToSameHeight(Ray ray) {
	return M_PI - ray.angle;
}

inline float rayAngleDownwards(Ray ray, float targetHeight) {
	float rayAngle = M_PI - asin(ray.height * sin(ray.angle) / targetHeight);
	return rayAngle;
}

// TODO make all parameters into flags
Atmosphere::Atmosphere()
:
	lambda(680e-9f, 550e-9f, 440e-9f), // Bruneton and Neyret
	earthRadius(6731.0e3f),
	atmosphereThickness(100.0e3f),
	rayleighThickness(7994.0f),
	mieThickness(1200.0f),
	rayleighCoefficient(5.8e-6f, 13.5e-6f, 33.1e-6f), // Bruneton and Neyret; they use n ~ 1.0003 apparently
	mieCoefficient(2e-6f), // 2e-5f // Bruneton and Neyret
	mieAbsorption(mieCoefficient / 9.0f)
{
}

float Atmosphere::rayleighDensityAtHeight(float height) const {
	return exp(-(height - earthRadius) / rayleighThickness);
}

float Atmosphere::mieDensityAtHeight(float height) const {
	return exp(-(height - earthRadius) / mieThickness);
}

AtmosphereLayers::Heights AtmosphereLayers::computeHeights(float earthRadius, float rayleighThickness, float atmosphereThickness) {
	Heights heights(numLayers);
	for (unsigned i = 0; i < numLayers - 1; ++i) {
		float const containedFraction = 1.0f - (float)i / (numLayers - 1);
		heights[i] = earthRadius - rayleighThickness * log(containedFraction);
	}
	heights[numLayers - 1] = std::max(heights[numLayers - 1] * 1.01f, earthRadius + atmosphereThickness);
	return heights;
}

// TODO make numAngles into flag
AtmosphereLayers::AtmosphereLayers(Atmosphere const &atmosphere, unsigned numLayers)
:
	numLayers(numLayers),
	numAngles(255),
	heights(computeHeights(atmosphere.earthRadius, atmosphere.rayleighThickness, atmosphere.atmosphereThickness))
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
	vec3 const extinction =
		rayleighExtinctionCoefficient * atmosphere.rayleighDensityAtHeight(ray.height) +
		mieExtinctionCoefficient * atmosphere.mieDensityAtHeight(ray.height);
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

void tableToTexture(Vec3Table2D const &table, GLTexture &texture) {
	BOOST_STATIC_ASSERT(sizeof(vec3) == 3 * sizeof(float));
	bindTexture(GL_TEXTURE_RECTANGLE, texture);
	glTexImage2D(GL_TEXTURE_RECTANGLE, 0, GL_RGBA32F, table.getSize().x, table.getSize().y, 0, GL_RGB, GL_FLOAT, (float const *)table.raw());
	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

Sky::Sky(Atmosphere const &atmosphere, AtmosphereLayers const &layers, Sun const *sun)
:
	atmosphere(atmosphere),
	layers(layers),
	sun(sun),
	transmittanceTable(buildTransmittanceTable(atmosphere, layers)),
	totalTransmittanceTable(buildTotalTransmittanceTable(atmosphere, layers, transmittanceTable))
{

	int const v[] = {
		-1, -1, -1,
		-1,  1, -1,
		-1,  1,  1,
		-1, -1,  1,

		 1, -1, -1,
		 1, -1,  1,
		 1,  1,  1,
		 1,  1, -1,

		-1, -1, -1,
		-1, -1,  1,
		 1, -1,  1,
		 1, -1, -1,

		-1,  1, -1,
		 1,  1, -1,
		 1,  1,  1,
		-1,  1,  1,

		-1, -1, -1,
		 1, -1, -1,
		 1,  1, -1,
		-1,  1, -1,
		
		-1, -1,  1,
		-1,  1,  1,
		 1,  1,  1,
		 1, -1,  1
	};
	vertices.putData(sizeof(v), v, GL_STATIC_DRAW);

	tableToTexture(transmittanceTable, transmittanceTexture);
	tableToTexture(totalTransmittanceTable, totalTransmittanceTexture);

	shaderProgram.loadAndLink("shaders/sky.vert", "shaders/sky.frag");
}

void Sky::update(float dt) {
}

void Sky::render() {
	unsigned const numLayers = layers.numLayers;
	unsigned const numAngles = layers.numAngles;

	glDisable(GL_DEPTH_TEST);
	glDepthMask(GL_FALSE);

	glEnableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);	

	bindBuffer(GL_ARRAY_BUFFER, vertices);
	glVertexPointer(3, GL_INT, 0, 0);

	useProgram(shaderProgram.getProgram());

	shaderProgram.setUniform("atmosphere.earthRadius", (float)atmosphere.earthRadius);
	shaderProgram.setUniform("atmosphere.rayleighThickness", (float)atmosphere.rayleighThickness);
	shaderProgram.setUniform("atmosphere.mieThickness", (float)atmosphere.mieThickness);
	shaderProgram.setUniform("atmosphere.rayleighCoefficient", vec3(atmosphere.rayleighCoefficient));
	shaderProgram.setUniform("atmosphere.mieCoefficient", vec3(atmosphere.mieCoefficient));
	shaderProgram.setUniform("sun.angularRadius", (float)sun->getAngularRadius());
	shaderProgram.setUniform("sun.color", sun->getColor());
	shaderProgram.setUniform("sun.direction", sun->getDirection());
	shaderProgram.setUniform("layers.numLayers", (int)numLayers);
	shaderProgram.setUniform("layers.numAngles", (int)numAngles);
	std::vector<float> heights(numLayers);
	for (unsigned i = 0; i < numLayers; ++i) {
		heights[i] = layers.heights[i];
	}
	shaderProgram.setUniform("layers.heights", heights);

	activeTexture(1);
	bindTexture(GL_TEXTURE_RECTANGLE, totalTransmittanceTexture);
	activeTexture(0);
	bindTexture(GL_TEXTURE_RECTANGLE, transmittanceTexture);
	shaderProgram.setUniform("transmittanceSampler", 0);
	shaderProgram.setUniform("totalTransmittanceSampler", 1);

	bindFragDataLocation(shaderProgram.getProgram(), 0, "scatteredLight");

	glDrawArrays(GL_QUADS, 0, vertices.getSizeInBytes() / sizeof(int) / 3);

	useFixedProcessing();

	glDepthMask(GL_TRUE);
	glEnable(GL_DEPTH_TEST);
}
