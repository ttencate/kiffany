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

bool rayHitsHeight(Ray ray, double targetHeight) {
	return ray.height * sin(ray.angle) < targetHeight;
	// return pow2(cos(startAngle)) >= 1.0 - pow2((earthRadius + targetHeight) / (earthRadius + startHeight));
}

double rayLengthUpwards(Ray ray, double targetHeight) {
	BOOST_ASSERT(ray.height <= targetHeight);
	BOOST_ASSERT(ray.angle <= 0.5 * M_PI);
	double cosAngle = cos(ray.angle);
	double rayLength = sqrt(
			sqr(ray.height) * (sqr(cosAngle) - 1.0) +
			sqr(targetHeight))
		- ray.height * cosAngle;
	BOOST_ASSERT(rayLength >= 0);
	return rayLength;
}

double rayLengthToSameHeight(Ray ray) {
	BOOST_ASSERT(ray.angle > 0.5 * M_PI);
	double rayLength = -2.0 * ray.height * cos(ray.angle);
	BOOST_ASSERT(rayLength >= 0);
	return rayLength;
}

double rayLengthDownwards(Ray ray, double targetHeight) {
	BOOST_ASSERT(targetHeight <= ray.height);
	BOOST_ASSERT(ray.angle > 0.5 * M_PI);
	BOOST_ASSERT(rayHitsHeight(ray, targetHeight));
	double cosAngle = cos(ray.angle);
	double rayLength = -sqrt(
			sqr(ray.height) * (sqr(cosAngle) - 1.0) +
			sqr(targetHeight))
		- ray.height * cosAngle;
	BOOST_ASSERT(rayLength >= 0);
	return rayLength;
}

double rayAngleUpwards(Ray ray, double targetHeight) {
	BOOST_ASSERT(ray.height <= targetHeight);
	BOOST_ASSERT(ray.angle <= 0.5 * M_PI);
	double rayAngle = asin(ray.height * sin(ray.angle) / targetHeight);
	BOOST_ASSERT(rayAngle >= 0);
	BOOST_ASSERT(rayAngle <= 0.5 * M_PI);
	return rayAngle;
}

double rayAngleToSameHeight(Ray ray) {
	BOOST_ASSERT(ray.angle >= 0.5 * M_PI);
	BOOST_ASSERT(ray.angle <= M_PI);
	return M_PI - ray.angle;
}

double rayAngleDownwards(Ray ray, double targetHeight) {
	BOOST_ASSERT(targetHeight <= ray.height);
	BOOST_ASSERT(ray.angle > 0.5 * M_PI);
	BOOST_ASSERT(rayHitsHeight(ray, targetHeight));
	double rayAngle = M_PI - asin(ray.height * sin(ray.angle) / targetHeight);
	BOOST_ASSERT(rayAngle >= 0.5 * M_PI);
	BOOST_ASSERT(rayAngle <= M_PI);
	return rayAngle;
}

//dvec3 const Scattering::lambda = dvec3(700e-9, 530e-9, 400e-9); // wavelengths of RGB (metres)
dvec3 const Scattering::lambda = dvec3(680e-9, 550e-9, 440e-9); // Bruneton and Neyret

Scattering::Scattering(double unityHeight, double thickness, dvec3 coefficient)
:
	unityHeight(unityHeight),
	thickness(thickness),
	coefficient(coefficient)
{
}

double Scattering::densityAtHeight(double height) const {
	return exp(-(height - unityHeight) / thickness);
}

dvec3 RayleighScattering::computeCoefficient() const {
	return dvec3(5.8e-6, 13.5e-6, 33.1e-6); // Bruneton and Neyret; they use n ~ 1.0003 apparently

	double const n = 1.000293; // index of refraction of air
	double const Ns = 2.545e25; // number density in standard atmosphere (molecules/m^3)
	double const K = 2.0 * M_PI * M_PI * sqr(sqr(n) - 1.0) / (3.0 * Ns);
	return K / pow(lambda, dvec3(4.0));
}

RayleighScattering::RayleighScattering(double unityHeight)
:
	Scattering(unityHeight, 7994, computeCoefficient())
{
}

double RayleighScattering::phaseFunction(double lightAngle) const {
	// Bruneton and Neyret
	double const mu = cos(lightAngle);
	return
		3.0 / (16.0 * M_PI)
		* (1.0 + sqr(mu));
}

dvec3 MieScattering::computeCoefficient() const {
	return dvec3(2e-5); // Bruneton and Neyret
}

MieScattering::MieScattering(double unityHeight)
:
	Scattering(unityHeight, 1200, computeCoefficient()),
	absorption(dvec3(coefficient / 9.0))
{
}

double MieScattering::phaseFunction(double lightAngle) const {
	// Bruneton and Neyret
	double const mu = cos(lightAngle);
	double const g = 0.76; // 0.76
	return 3.0 / (8.0 * M_PI) *
		(1 - pow2(g)) * (1 + pow2(mu)) /
		((2 + pow2(g)) * pow(1 + pow2(g) - 2.0 * g * mu, 3.0 / 2.0));

	/*
	double const u = 0.70; // Mie scattering parameter (0.7-0.85)
	double const x =
		5.0/9.0 * u
		+ 125.0/729.0 * u*u*u
		+ sqrt(64.0/27.0 - 325.0/243.0 * u*u + 1250.0/2187.0 * u*u*u*u);
	double const g =
		5.0/9.0 * u
		- (4.0/3.0 - 25.0/81.0 * u*u) * pow(x, -1.0/3.0)
		+ pow(x, 1.0/3.0);
	// -1 <= g <= 1, backscattering through isotropic through forward
	return
		(1 - sqr(g))
		/
		(4.0 * M_PI * pow(1 - 2.0 * g * cos(lightAngle) + sqr(g), 3.0/2.0));
		*/
}

Atmosphere::Atmosphere(double earthRadius, double thickness)
:
	earthRadius(earthRadius),
	thickness(thickness),
	rayleighScattering(earthRadius),
	mieScattering(earthRadius)
{
}

AtmosphereLayers::Heights AtmosphereLayers::computeHeights(double earthRadius, double thickness, double atmosphereThickness) {
	Heights heights(numLayers);
	for (unsigned i = 0; i < numLayers - 1; ++i) {
		double const containedFraction = 1.0f - (double)i / (numLayers - 1);
		heights[i] = earthRadius - thickness * log(containedFraction);
	}
	heights[numLayers - 1] = std::max(heights[numLayers - 1] * 1.01f, earthRadius + atmosphereThickness);
	return heights;
}

AtmosphereLayers::AtmosphereLayers(Atmosphere const &atmosphere, unsigned numLayers)
:
	numLayers(numLayers),
	numAngles(256),
	heights(computeHeights(atmosphere.earthRadius, atmosphere.rayleighScattering.thickness, atmosphere.thickness))
{
}

// Ray length from the given layer to the next,
// where 'next' might be the one above (for angle <= 0.5pi),
// the layer itself (for 0.5pi < angle < x),
// or the layer below it (for x <= angle).
inline double rayLengthToNextLayer(Ray ray, AtmosphereLayers const &layers, unsigned layer) {
	if (ray.angle <= 0.5 * M_PI && layer == layers.numLayers - 1) {
		// Ray goes up into space
		return 0.0;
	} else if (ray.angle <= 0.5 * M_PI) {
		// Ray goes up, hits layer above
		return rayLengthUpwards(ray, layers.heights[layer + 1]);
	} else if (layer == 0) {
		// Ray goes down, into the ground
		return 0.0;
	} else if (!rayHitsHeight(ray, layers.heights[layer - 1])) {
		// Ray goes down, misses layer below, hits current layer from below
		return rayLengthToSameHeight(ray);
	} else {
		// Ray goes down, hits layer below
		return rayLengthDownwards(ray, layers.heights[layer - 1]);
	}
}

inline dvec3 transmittanceToNextLayer(Ray ray, Atmosphere const &atmosphere, AtmosphereLayers const &layers, unsigned layer) {
	dvec3 const rayleighExtinctionCoefficient = atmosphere.rayleighScattering.coefficient;
	dvec3 const mieExtinctionCoefficient = atmosphere.mieScattering.coefficient + atmosphere.mieScattering.absorption;

	double rayLength = rayLengthToNextLayer(ray, layers, layer);
	dvec3 const extinction =
		rayleighExtinctionCoefficient * atmosphere.rayleighScattering.densityAtHeight(ray.height) +
		mieExtinctionCoefficient * atmosphere.mieScattering.densityAtHeight(ray.height);
	return exp(-extinction * rayLength);
}

Dvec3Table2D buildTransmittanceTable(Atmosphere const &atmosphere, AtmosphereLayers const &layers) {
	unsigned const numAngles = layers.numAngles;
	unsigned const numLayers = layers.numLayers;

	Dvec3Table2D transmittanceTable = Dvec3Table2D::createWithCoordsSizeAndOffset(
			uvec2(numLayers, numAngles),
			dvec2(numLayers, M_PI * numAngles / (numAngles - 1)),
			dvec2(0.0, 0.0));
	for (unsigned a = 0; a < numAngles; ++a) {
		double const angle = M_PI * a / (numAngles - 1);
		for (unsigned layer = 0; layer < numLayers; ++layer) {
			double const height = layers.heights[layer];
			Ray ray(height, angle);
			dvec3 transmittance = transmittanceToNextLayer(ray, atmosphere, layers, layer);
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
Dvec3Table2D buildTotalTransmittanceTable(Atmosphere const &atmosphere, AtmosphereLayers const &layers, Dvec3Table2D const &transmittanceTable) {
	unsigned const numAngles = layers.numAngles;
	unsigned const numLayers = layers.numLayers;

	Dvec3Table2D totalTransmittanceTable = Dvec3Table2D::createWithCoordsSizeAndOffset(
			uvec2(numLayers, numAngles),
			dvec2(numLayers, M_PI * numAngles / (numAngles - 1)),
			dvec2(0.0, 0.0));
	// For upward angles, cumulatively sum over all layers.
	for (int layer = numLayers - 1; layer >= 0; --layer) {
		for (unsigned a = 0; a < numAngles / 2; ++a) {
			double const angle = M_PI * a / (numAngles - 1);
			Ray ray(layers.heights[layer], angle);
			dvec3 totalTransmittance;
			if (layer == (int)numLayers - 1) {
				// Ray goes directly into space
				totalTransmittance = dvec3(1.0);
			} else {
				// Ray passes through some layers
				double const nextAngle = rayAngleUpwards(ray, layers.heights[layer + 1]);
				BOOST_ASSERT(nextAngle <= 0.5 * M_PI);
				
				totalTransmittance =
					transmittanceTable(dvec2(layer, angle)) *
					totalTransmittanceTable(dvec2(layer + 1, nextAngle));
			}
			totalTransmittanceTable.set(uvec2(layer, a), totalTransmittance);
		}
	}
	// We can only do downward angles after we've got all upward ones,
	// because (unless they hit the ground) they eventually start going up again.
	// Also, we need to do them in bottom-to-top order.
	for (unsigned layer = 0; layer < numLayers; ++layer) {
		for (unsigned a = numAngles / 2; a < numAngles; ++a) {
			double const angle = M_PI * a / (numAngles - 1);
			Ray ray(layers.heights[layer], angle);
			dvec3 totalTransmittance;
			if (layer == 0) {
				// Ray goes directly into the ground
				totalTransmittance = dvec3(0.0);
			} else if (rayHitsHeight(ray, layers.heights[layer - 1])) {
				// Ray hits the layer below
				double const nextAngle = rayAngleDownwards(ray, layers.heights[layer - 1]);
				BOOST_ASSERT(nextAngle >= 0.5 * M_PI);
				totalTransmittance =
					transmittanceTable(dvec2(layer, angle)) *
					totalTransmittanceTable(dvec2(layer - 1, nextAngle));
			} else {
				// Ray misses the layer below, hits the current one from below
				double const nextAngle = rayAngleToSameHeight(ray);
				BOOST_ASSERT(nextAngle <= 0.5 * M_PI);
				totalTransmittance =
					transmittanceTable(dvec2(layer, angle)) *
					totalTransmittanceTable(dvec2(layer, nextAngle));
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

Scatterer::Scatterer(Atmosphere const &atmosphere, AtmosphereLayers const &layers)
:
	atmosphere(atmosphere),
	layers(layers),
	transmittanceTable(buildTransmittanceTable(atmosphere, layers)),
	totalTransmittanceTable(buildTotalTransmittanceTable(atmosphere, layers, transmittanceTable))
{
}

inline dvec3 Scatterer::scatteredLight(dvec3 viewDirection, Sun const &sun) const {
	if (viewDirection.z < 0.0) {
		return dvec3(0.0);
	}
	double const sunAngularRadius = sun.getAngularRadius();
	dvec3 sunColor(sun.getColor());
	dvec3 sunDirection(sun.getDirection());

	double const lightAngle = acos(dot(viewDirection, sunDirection));
	double const groundViewAngle = acos(viewDirection.z);
	double const groundSunAngle = acos(sunDirection.z);
	Ray groundViewRay(atmosphere.earthRadius, groundViewAngle);
	Ray groundSunRay(atmosphere.earthRadius, groundSunAngle);

	double const rayleighPhaseFunction = atmosphere.rayleighScattering.phaseFunction(lightAngle);
	double const miePhaseFunction = atmosphere.mieScattering.phaseFunction(lightAngle);

	dvec3 scatteredLight = (1.0 - smoothstep(sunAngularRadius, sunAngularRadius * 1.2, lightAngle)) * sunColor;
	for (int layer = layers.numLayers - 2; layer >= 0; --layer) {
		double const height = layers.heights[layer];

		Ray viewRay(height, rayAngleUpwards(groundViewRay, height));
		BOOST_ASSERT(viewRay.angle <= 0.5 * M_PI);
		double const rayLength = rayLengthUpwards(viewRay, layers.heights[layer + 1]);

		double const sunAngle = rayAngleUpwards(groundSunRay, height);

		// Add inscattering, attenuated by optical depth to the sun
		dvec3 const rayleighInscattering =
			atmosphere.rayleighScattering.coefficient *
			atmosphere.rayleighScattering.densityAtHeight(height) *
			rayleighPhaseFunction;
		dvec3 const mieInscattering =
			atmosphere.mieScattering.coefficient *
			atmosphere.mieScattering.densityAtHeight(height) *
			miePhaseFunction;
		dvec3 const transmittance = totalTransmittanceTable(dvec2(layer, sunAngle));
		scatteredLight += rayLength * sunColor * transmittance * (rayleighInscattering + mieInscattering);

		// Multiply transmittance
		dvec3 const layerTransmittance = transmittanceTable(dvec2(layer, viewRay.angle));
		scatteredLight *= layerTransmittance;
	}
	return scatteredLight;
}

Sky::Sky(Scatterer const &scatterer, Sun const *sun)
:
	scatterer(scatterer),
	sun(sun),
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

	generateFaces();

	shaderProgram.loadAndLink("shaders/sky.vert", "shaders/sky.frag");
}

dvec3 Sky::computeColor(vec3 direction) {
	return scatterer.scatteredLight(dvec3(direction), *sun);

	/*
	// Super simple scheme by ATI (Preetham et al.)
	double cost = dot(sunDirection, direction);
	double g = 0.85;
	dvec3 br = dvec3(5.8e-6, 13.5e-6, 33.1e-6); // Bruneton and Neyret; they use n ~ 1.0003 apparently
	dvec3 bm = dvec3(2e-5); // Bruneton and Neyret
	dvec3 brt = 3.0 / (16.0 * M_PI) * br * (1 + pow2(cost));
	dvec3 bmt = 1.0 / (4.0 * M_PI) * bm * pow2(1 - g) / pow(1 + pow2(g) - 2.0 * g * cost, 3.0 / 2.0);
	double s = 100e3 / direction.z;
	dvec3 lin = (brt + bmt) / (br + bm) * sunColor * (1.0 - exp(-(bm + br) * s));
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
			vec3 color = clamp(vec3(computeColor(direction)), 0.0f, 1.0f);
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
	glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);
	glDepthMask(GL_FALSE);
	glEnable(GL_TEXTURE_CUBE_MAP);

	glEnableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);	
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	bindBuffer(GL_ARRAY_BUFFER, vertices);
	glVertexPointer(3, GL_INT, 0, 0);
	glTexCoordPointer(3, GL_INT, 0, 0);

	bindTexture(GL_TEXTURE_CUBE_MAP, texture);

	useProgram(shaderProgram);

	glDrawArrays(GL_QUADS, 0, vertices.getSizeInBytes() / sizeof(int) / 3);

	useFixedProcessing();

	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisable(GL_TEXTURE_CUBE_MAP);
	glDepthMask(GL_TRUE);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
}
