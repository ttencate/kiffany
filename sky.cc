#include "sky.h"

#include "shader.h"

#include <boost/assert.hpp>

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

bool rayHitsHeight(Ray ray, float targetHeight) {
	return ray.height * sin(ray.angle) < targetHeight;
	// return pow2(cos(startAngle)) >= 1.0f - pow2((earthRadius + targetHeight) / (earthRadius + startHeight));
}

float rayLengthUpwards(Ray ray, float targetHeight) {
	BOOST_ASSERT(ray.height <= targetHeight);
	BOOST_ASSERT(ray.angle <= 0.5f * M_PI);
	float cosAngle = cos(ray.angle);
	float rayLength = sqrt(
			sqr(ray.height) * (sqr(cosAngle) - 1.0f) +
			sqr(targetHeight))
		- ray.height * cosAngle;
	BOOST_ASSERT(rayLength >= 0.0f);
	return rayLength;
}

float rayLengthToSameHeight(Ray ray) {
	BOOST_ASSERT(ray.angle > 0.5f * M_PI);
	float rayLength = -2.0f * ray.height * cos(ray.angle);
	BOOST_ASSERT(rayLength >= 0.0f);
	return rayLength;
}

float rayLengthDownwards(Ray ray, float targetHeight) {
	BOOST_ASSERT(targetHeight <= ray.height);
	BOOST_ASSERT(ray.angle > 0.5f * M_PI);
	BOOST_ASSERT(rayHitsHeight(ray, targetHeight));
	float cosAngle = cos(ray.angle);
	float rayLength = -sqrt(
			sqr(ray.height) * (sqr(cosAngle) - 1.0f) +
			sqr(targetHeight))
		- ray.height * cosAngle;
	BOOST_ASSERT(rayLength >= 0.0f);
	return rayLength;
}

float rayAngleUpwards(Ray ray, float targetHeight) {
	BOOST_ASSERT(ray.height <= targetHeight);
	BOOST_ASSERT(ray.angle <= 0.5f * M_PI);
	float rayAngle = asin(ray.height * sin(ray.angle) / targetHeight);
	BOOST_ASSERT(rayAngle >= 0.0f);
	BOOST_ASSERT(rayAngle <= 0.5f * M_PI);
	return rayAngle;
}

float rayAngleToSameHeight(Ray ray) {
	BOOST_ASSERT(ray.angle >= 0.5f * M_PI);
	BOOST_ASSERT(ray.angle <= M_PI);
	return M_PI - ray.angle;
}

float rayAngleDownwards(Ray ray, float targetHeight) {
	BOOST_ASSERT(targetHeight <= ray.height);
	BOOST_ASSERT(ray.angle > 0.5f * M_PI);
	BOOST_ASSERT(rayHitsHeight(ray, targetHeight));
	float rayAngle = M_PI - asin(ray.height * sin(ray.angle) / targetHeight);
	BOOST_ASSERT(rayAngle >= 0.5f * M_PI);
	// BOOST_ASSERT(rayAngle <= M_PI); TODO fix for --start_time 5.4
	return rayAngle;
}

//vec3 const Scattering::lambda = vec3(700e-9, 530e-9, 400e-9); // wavelengths of RGB (metres)
vec3 const Scattering::lambda = vec3(680e-9f, 550e-9f, 440e-9f); // Bruneton and Neyret

Scattering::Scattering(float unityHeight, float thickness, vec3 coefficient)
:
	unityHeight(unityHeight),
	thickness(thickness),
	coefficient(coefficient)
{
}

float Scattering::densityAtHeight(float height) const {
	return exp(-(height - unityHeight) / thickness);
}

vec3 RayleighScattering::computeCoefficient() const {
	return vec3(5.8e-6, 13.5e-6, 33.1e-6); // Bruneton and Neyret; they use n ~ 1.0f003 apparently

	float const n = 1.000293f; // index of refraction of air
	float const Ns = 2.545e25f; // number density in standard atmosphere (molecules/m^3)
	float const K = 2.0f * M_PI * M_PI * sqr(sqr(n) - 1.0f) / (3.0f * Ns);
	return K / pow(lambda, vec3(4.0f));
}

RayleighScattering::RayleighScattering(float unityHeight)
:
	Scattering(unityHeight, 7994, computeCoefficient())
{
}

float RayleighScattering::phaseFunction(float lightAngle) const {
	// Bruneton and Neyret
	float const mu = cos(lightAngle);
	return
		3.0f / (16.0f * M_PI)
		* (1.0f + sqr(mu));
}

vec3 MieScattering::computeCoefficient() const {
	return vec3(2e-6f); // vec3(2e-5f); // Bruneton and Neyret
}

MieScattering::MieScattering(float unityHeight)
:
	Scattering(unityHeight, 1200.0f, computeCoefficient()),
	absorption(vec3(coefficient / 9.0f))
{
}

float MieScattering::phaseFunction(float lightAngle) const {
	// Bruneton and Neyret
	float const mu = cos(lightAngle);
	float const g = 0.76f; // 0.76
	return 3.0f / (8.0f * M_PI) *
		(1.0f - pow2(g)) * (1.0f + pow2(mu)) /
		((2.0f + pow2(g)) * pow(1.0f + pow2(g) - 2.0f * g * mu, 3.0f / 2.0f));

	/*
	float const u = 0.70; // Mie scattering parameter (0.7-0.85)
	float const x =
		5.0f/9.0f * u
		+ 125.0f/729.0f * u*u*u
		+ sqrt(64.0f/27.0f - 325.0f/243.0f * u*u + 1250.0f/2187.0f * u*u*u*u);
	float const g =
		5.0f/9.0f * u
		- (4.0f/3.0f - 25.0f/81.0f * u*u) * pow(x, -1.0f/3.0f)
		+ pow(x, 1.0f/3.0f);
	// -1 <= g <= 1, backscattering through isotropic through forward
	return
		(1 - sqr(g))
		/
		(4.0f * M_PI * pow(1 - 2.0f * g * cos(lightAngle) + sqr(g), 3.0f/2.0f));
		*/
}

Atmosphere::Atmosphere(float earthRadius, float thickness)
:
	earthRadius(earthRadius),
	thickness(thickness),
	rayleighScattering(earthRadius),
	mieScattering(earthRadius)
{
}

AtmosphereLayers::Heights AtmosphereLayers::computeHeights(float earthRadius, float thickness, float atmosphereThickness) {
	Heights heights(numLayers);
	for (unsigned i = 0; i < numLayers - 1; ++i) {
		float const containedFraction = 1.0f - (float)i / (numLayers - 1);
		heights[i] = earthRadius - thickness * log(containedFraction);
	}
	heights[numLayers - 1] = std::max(heights[numLayers - 1] * 1.01f, earthRadius + atmosphereThickness);
	return heights;
}

AtmosphereLayers::AtmosphereLayers(Atmosphere const &atmosphere, unsigned numLayers)
:
	numLayers(numLayers),
	numAngles(255),
	heights(computeHeights(atmosphere.earthRadius, atmosphere.rayleighScattering.thickness, atmosphere.thickness))
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
	vec3 const rayleighExtinctionCoefficient = atmosphere.rayleighScattering.coefficient;
	vec3 const mieExtinctionCoefficient = atmosphere.mieScattering.coefficient + atmosphere.mieScattering.absorption;

	float rayLength = rayLengthToNextLayer(ray, layers, layer);
	vec3 const extinction =
		rayleighExtinctionCoefficient * atmosphere.rayleighScattering.densityAtHeight(ray.height) +
		mieExtinctionCoefficient * atmosphere.mieScattering.densityAtHeight(ray.height);
	return exp(-extinction * rayLength);
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

	/*
	std::cout << "Transmittance table:\n";
	std::cout << std::fixed;
	std::cout.precision(5);
	for (int layer = numLayers - 1; layer >= 0; --layer) {
		std::cout << layer << "  ";
		for (unsigned a = 0; a < numAngles; ++a) {
			if (a == numAngles / 2) {
				std::cout << "| ";
			}
			std::cout << transmittanceTable.get(uvec2(layer, a)).r << ' ';
		}
		std::cout << '\n';
	}
	*/
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
				BOOST_ASSERT(nextAngle <= 0.5f * M_PI);
				
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
				BOOST_ASSERT(nextAngle >= 0.5f * M_PI);
				totalTransmittance =
					transmittanceTable(vec2(layer, angle)) *
					totalTransmittanceTable(vec2(layer - 1, nextAngle));
			} else {
				// Ray misses the layer below, hits the current one from below
				float const nextAngle = rayAngleToSameHeight(ray);
				BOOST_ASSERT(nextAngle <= 0.5f * M_PI);
				totalTransmittance =
					transmittanceTable(vec2(layer, angle)) *
					totalTransmittanceTable(vec2(layer, nextAngle));
			}
			totalTransmittanceTable.set(uvec2(layer, a), totalTransmittance);
		}
	}

	/*
	std::cout << "Total transmittance table:\n";
	std::cout << std::fixed;
	std::cout.precision(5);
	for (int layer = numLayers - 1; layer >= 0; --layer) {
		std::cout << layer << "  ";
		for (unsigned a = 0; a < numAngles; ++a) {
			if (a == numAngles / 2) {
				std::cout << "| ";
			}
			std::cout << totalTransmittanceTable.get(uvec2(layer, a)).r << ' ';
		}
		std::cout << '\n';
	}
	*/
	return totalTransmittanceTable;
}

void tableToTexture(Vec3Table2D const &table, GLTexture &texture) {
	unsigned width = table.getSize().x;
	unsigned height = table.getSize().y;
	std::vector<float> data(3 * width * height);
	float *q = &data[0];
	for (vec3 const *p = table.begin(); p < table.end(); ++p) {
		*(q++) = p->r;
		*(q++) = p->g;
		*(q++) = p->b;
	}
	bindTexture(GL_TEXTURE_RECTANGLE, texture);
	glTexImage2D(GL_TEXTURE_RECTANGLE, 0, GL_RGBA32F, table.getSize().x, table.getSize().y, 0, GL_RGB, GL_FLOAT, &data[0]);
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
	totalTransmittanceTable(buildTotalTransmittanceTable(atmosphere, layers, transmittanceTable)),
	size(128),
	textureImage(new unsigned char[size * size * 3])
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

	bindTexture(GL_TEXTURE_CUBE_MAP, texture);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	tableToTexture(transmittanceTable, transmittanceTexture);
	tableToTexture(totalTransmittanceTable, totalTransmittanceTexture);

	generateFaces();

	shaderProgram.loadAndLink("shaders/sky.vert", "shaders/sky.frag");
}

vec3 Sky::computeColor(vec3 viewDirection) {
	if (viewDirection.z < 0.0f) {
		return vec3(0.0f);
	}
	float const sunAngularRadius = sun->getAngularRadius();
	vec3 sunColor(sun->getColor());
	vec3 sunDirection(sun->getDirection());

	float const lightAngle = acos(dot(viewDirection, sunDirection));
	float const groundViewAngle = acos(viewDirection.z);
	float const groundSunAngle = acos(sunDirection.z);
	Ray groundViewRay(atmosphere.earthRadius, groundViewAngle);
	Ray groundSunRay(atmosphere.earthRadius, groundSunAngle);

	float const rayleighPhaseFunction = atmosphere.rayleighScattering.phaseFunction(lightAngle);
	float const miePhaseFunction = atmosphere.mieScattering.phaseFunction(lightAngle);

	vec3 scatteredLight = (1.0f - smoothstep(sunAngularRadius, sunAngularRadius * 1.2f, lightAngle)) * sunColor;
	for (int layer = layers.numLayers - 2; layer >= 0; --layer) {
		float const height = layers.heights[layer];

		Ray viewRay(height, rayAngleUpwards(groundViewRay, height));
		BOOST_ASSERT(viewRay.angle <= 0.5f * M_PI);
		float const rayLength = rayLengthUpwards(viewRay, layers.heights[layer + 1]);

		// TODO broken!
		float const sunAngle = groundSunAngle;// rayAngleUpwards(groundSunRay, height);

		// Add inscattering, attenuated by optical depth to the sun
		vec3 const rayleighInscattering =
			atmosphere.rayleighScattering.coefficient *
			atmosphere.rayleighScattering.densityAtHeight(height) *
			rayleighPhaseFunction;
		vec3 const mieInscattering =
			atmosphere.mieScattering.coefficient *
			atmosphere.mieScattering.densityAtHeight(height) *
			miePhaseFunction;
		vec3 const transmittance = totalTransmittanceTable(vec2(layer, sunAngle));
		scatteredLight += rayLength * sunColor * transmittance * (rayleighInscattering + mieInscattering);

		// Multiply transmittance
		vec3 const layerTransmittance = transmittanceTable(vec2(layer, viewRay.angle));
		scatteredLight *= layerTransmittance;
	}
	return scatteredLight;

	/*
	// Super simple scheme by ATI (Preetham et al.)
	float cost = dot(sunDirection, direction);
	float g = 0.85;
	vec3 br = vec3(5.8e-6, 13.5e-6, 33.1e-6); // Bruneton and Neyret; they use n ~ 1.0f003 apparently
	vec3 bm = vec3(2e-5); // Bruneton and Neyret
	vec3 brt = 3.0f / (16.0f * M_PI) * br * (1 + pow2(cost));
	vec3 bmt = 1.0f / (4.0f * M_PI) * bm * pow2(1 - g) / pow(1 + pow2(g) - 2.0f * g * cost, 3.0f / 2.0f);
	float s = 100e3 / direction.z;
	vec3 lin = (brt + bmt) / (br + bm) * sunColor * (1.0f - exp(-(bm + br) * s));
	return lin;
	*/
}

void Sky::generateFace(GLenum face, vec3 base, vec3 xBasis, vec3 yBasis) {
	unsigned char *p = textureImage.get();
	for (unsigned y = 0; y < size; ++y) {
		for (unsigned x = 0; x < size; ++x) {
			vec3 direction = normalize(
				base +
				(x + 0.5f) / size * xBasis +
				(y + 0.5f) / size * yBasis);
			vec3 color = clamp(computeColor(direction), 0.0f, 1.0f);
			p[0] = (unsigned char)(0xFF * color[0]);
			p[1] = (unsigned char)(0xFF * color[1]);
			p[2] = (unsigned char)(0xFF * color[2]);
			p += 3;
		}
	}
	glTexImage2D(face, 0, GL_RGB, size, size, 0, GL_RGB, GL_UNSIGNED_BYTE, textureImage.get());
}

void Sky::generateFaces() {
	float const N = -0.5f;
	float const P = 0.5f;
	bindTexture(GL_TEXTURE_CUBE_MAP, texture);
	generateFace(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, vec3(N, P, N), vec3(0, 0, 1), vec3(0, -1, 0));
	generateFace(GL_TEXTURE_CUBE_MAP_POSITIVE_X, vec3(P, P, P), vec3(0, 0, -1), vec3(0, -1, 0));
	generateFace(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, vec3(N, N, P), vec3(1, 0, 0), vec3(0, 0, -1));
	generateFace(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, vec3(N, P, N), vec3(1, 0, 0), vec3(0, 0, 1));
	generateFace(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, vec3(P, P, N), vec3(-1, 0, 0), vec3(0, -1, 0));
	generateFace(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, vec3(N, P, P), vec3(1, 0, 0), vec3(0, -1, 0));
}

void Sky::update(float dt) {
}

void Sky::render() {
	unsigned const numLayers = layers.numLayers;
	unsigned const numAngles = layers.numAngles;

	glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);
	glDepthMask(GL_FALSE);

	glEnableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);	

	bindBuffer(GL_ARRAY_BUFFER, vertices);
	glVertexPointer(3, GL_INT, 0, 0);

	useProgram(shaderProgram.getProgram());

	shaderProgram.setUniform("atmosphere.earthRadius", (float)atmosphere.earthRadius);
	shaderProgram.setUniform("atmosphere.rayleighThickness", (float)atmosphere.rayleighScattering.thickness);
	shaderProgram.setUniform("atmosphere.mieThickness", (float)atmosphere.mieScattering.thickness);
	shaderProgram.setUniform("atmosphere.rayleighScatteringCoefficient", vec3(atmosphere.rayleighScattering.coefficient));
	shaderProgram.setUniform("atmosphere.mieScatteringCoefficient", vec3(atmosphere.mieScattering.coefficient));
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
	glEnable(GL_LIGHTING);
}
