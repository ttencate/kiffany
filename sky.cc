#include "sky.h"

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

bool rayHitsHeight(double startHeight, double targetHeight, double startAngle, double earthRadius) {
	return pow2(cos(startAngle)) >= 1.0 - pow2((earthRadius + targetHeight) / (earthRadius + startHeight));
}

double rayLengthBetweenHeights(double startHeight, double targetHeight, double startAngle, double earthRadius) {
	double cosAngle = cos(startAngle);
	return /*+-*/sqrt(
			sqr(earthRadius + startHeight) * (sqr(cosAngle) - 1.0) +
			sqr(earthRadius + targetHeight)) -
		(earthRadius + startHeight) * cosAngle;
}

double rayAngleAtHeight(double startHeight, double targetHeight, double startAngle, double earthRadius) {
	return asin((earthRadius + startHeight) * sin(startAngle) / (earthRadius + targetHeight));
}

//dvec3 const Scattering::lambda = dvec3(700e-9, 530e-9, 400e-9); // wavelengths of RGB (metres)
dvec3 const Scattering::lambda = dvec3(680e-9, 550e-9, 440e-9); // Bruneton and Neyret

Scattering::Scattering(double height, dvec3 coefficient)
:
	height(height),
	coefficient(coefficient)
{
}

double Scattering::densityAtHeight(double height) const {
	return exp(-height / this->height);
}

double Scattering::opticalLengthBetweenHeights(double lowerHeight, double upperHeight, double lowerAngle, double earthRadius) const {
	// densityAtHeight, integrated from lowerHeight to upperHeight
	double length = rayLengthBetweenHeights(lowerHeight, upperHeight, lowerAngle, earthRadius);
	return length * 0.5 * (densityAtHeight(lowerHeight) + densityAtHeight(upperHeight));
	
	// Preetham, page 2: assumes radius of Earth, presumably
	// double const zenithOpticalLength = height * (densityAtHeight(lowerHeight) - densityAtHeight(upperHeight));
	// double const relativeOpticalLength = 1.0 / (cos(lowerAngle) + 0.15 * pow(93.885 - lowerAngle, -1.253));
	// return relativeOpticalLength * zenithOpticalLength;
}

dvec3 RayleighScattering::computeCoefficient() const {
	return dvec3(5.8e-6, 13.5e-6, 33.1e-6); // Bruneton and Neyret; they use n ~ 1.0003 apparently

	double const n = 1.000293; // index of refraction of air
	double const Ns = 2.545e25; // number density in standard atmosphere (molecules/m^3)
	double const K = 2.0 * M_PI * M_PI * sqr(sqr(n) - 1.0) / (3.0 * Ns);
	return K / pow(lambda, dvec3(4.0));
}

RayleighScattering::RayleighScattering()
:
	Scattering(7994, computeCoefficient())
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
	return 0.1 * dvec3(2e-5); // Bruneton and Neyret

	// TODO why do we need a much smaller number than in the literature?
	double const c = 6e-22; // 6e-17 for clear, 25e-17 for overcast
	double const nu = 4.0;
	dvec3 K(0.68, 0.673, 0.663); // Preetham, Table 2
	return 0.434 * c * M_PI * pow(2 * M_PI / lambda, dvec3(nu - 2.0)) * K;
}

MieScattering::MieScattering()
:
	Scattering(1200, computeCoefficient()),
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

Atmosphere::Atmosphere(double earthRadius, double atmosphereHeight)
:
	earthRadius(earthRadius),
	atmosphereHeight(atmosphereHeight)
{
}

AtmosphereLayers::Heights AtmosphereLayers::computeHeights(double rayleighHeight, double atmosphereHeight) {
	Heights heights(numLayers);
	for (unsigned i = 0; i < numLayers - 1; ++i) {
		double const containedFraction = 1.0f - (double)i / (numLayers - 1);
		heights[i] = -rayleighHeight * log(containedFraction);
	}
	heights[numLayers - 1] = std::max(heights[numLayers - 1] * 1.01f, atmosphereHeight);
	return heights;
}

AtmosphereLayers::AtmosphereLayers(Atmosphere const &atmosphere, unsigned numLayers)
:
	numLayers(numLayers),
	numAngles(256),
	heights(computeHeights(atmosphere.rayleighScattering.height, atmosphere.atmosphereHeight))
{
}

// Transmittance between the given layer and the next,
// where 'next' might be the one above (for angle <= 0.5pi),
// the layer itself (for 0.5pi < angle < x),
// or the layer below it (for x <= angle).
Dvec3Table2D buildTransmittanceTable(Atmosphere const &atmosphere, AtmosphereLayers const &layers) {
	unsigned const numAngles = layers.numAngles;
	unsigned const numLayers = layers.numLayers;
	dvec3 const rayleighExtinctionCoefficient = atmosphere.rayleighScattering.coefficient;
	dvec3 const mieExtinctionCoefficient = atmosphere.mieScattering.coefficient + atmosphere.mieScattering.absorption;

	Dvec3Table2D transmittanceTable = Dvec3Table2D::createWithCoordsSizeAndOffset(
			uvec2(numLayers, numAngles),
			dvec2(numLayers, M_PI * numAngles / (numAngles - 1)),
			dvec2(0.0, 0.0));
	for (unsigned a = 0; a < numAngles; ++a) {
		double const angle = M_PI * a / (numAngles - 1);
		for (unsigned layer = 0; layer < numLayers; ++layer) {
			double const height = layers.heights[layer];
			double rayLength;
			if (angle <= 0.5 * M_PI && layer == numLayers - 1) {
				// Ray goes up into space
				rayLength = 0.0;
			} else if (angle <= 0.5 * M_PI) {
				// Ray goes up, hits layer above
				rayLength = rayLengthBetweenHeights(height, layers.heights[layer + 1], angle, atmosphere.earthRadius);
			} else if (layer == 0) {
				// Ray goes down, into the ground
				rayLength = INFINITY;
			} else if (!rayHitsHeight(height, layers.heights[layer - 1], angle, atmosphere.earthRadius)) {
				// Ray goes down, misses layer below, hits current layer from below
				rayLength = rayLengthBetweenHeights(height, height, angle, atmosphere.earthRadius);
			} else {
				// Ray goes down, hits layer below
				double const angleAtPrevious = rayAngleAtHeight(height, layers.heights[layer - 1], angle, atmosphere.earthRadius);
				rayLength = rayLengthBetweenHeights(layers.heights[layer - 1], height, angleAtPrevious, atmosphere.earthRadius);
			}
			dvec3 const extinction =
				rayleighExtinctionCoefficient * atmosphere.rayleighScattering.densityAtHeight(height) +
				mieExtinctionCoefficient * atmosphere.mieScattering.densityAtHeight(height);
			dvec3 transmittance = exp(-extinction * rayLength);
			transmittanceTable.set(uvec2(layer, a), transmittance);
		}
	}

	// std::cout << "Transmittance table:\n" << transmittanceTable << '\n';
	return transmittanceTable;
}

// Transmittance from the ground up to the given layer,
// for the zenith angle on the ground
Dvec3Table2D buildSunTransmittanceTable(Atmosphere const &atmosphere, AtmosphereLayers const &layers) {
	unsigned const numAngles = layers.numAngles;
	unsigned const numLayers = layers.numLayers;
	dvec3 const rayleighExtinctionCoefficient = atmosphere.rayleighScattering.coefficient;
	dvec3 const mieExtinctionCoefficient = atmosphere.mieScattering.coefficient + atmosphere.mieScattering.absorption;

	Dvec3Table2D sunTransmittanceTable = Dvec3Table2D::createWithCoordsSizeAndOffset(
			uvec2(numLayers, numAngles),
			dvec2(numLayers, M_PI * numAngles / (numAngles - 1)),
			dvec2(0.0, 0.0));
	for (unsigned a = 0; a < numAngles; ++a) {
		double const groundAngle = M_PI * a / (numAngles - 1);
		if (groundAngle > M_PI / 2) {
			// Ray goes into the ground
			for (unsigned layer = 0; layer < numLayers; ++layer) {
				sunTransmittanceTable.set(uvec2(layer, a), dvec3(0.0));
			}
		} else {
			// Cumulatively sum over all layers
			sunTransmittanceTable.set(uvec2(numLayers - 1, a), dvec3(1.0));
			for (int layer = numLayers - 2; layer >= 0; --layer) {
				double const lowerHeight = layers.heights[layer];
				double const upperHeight = layers.heights[layer + 1];
				// TODO the notion of groundAngle is broken: ray from in the air, sloping down but missing the ground, does not have one
				double const lowerAngle = rayAngleAtHeight(0.0, lowerHeight, groundAngle, atmosphere.earthRadius);
				double const rayLength = rayLengthBetweenHeights(lowerHeight, upperHeight, lowerAngle, atmosphere.earthRadius);
				dvec3 const extinction =
					rayleighExtinctionCoefficient * atmosphere.rayleighScattering.densityAtHeight(upperHeight) +
					mieExtinctionCoefficient * atmosphere.mieScattering.densityAtHeight(upperHeight);
				dvec3 const transmittance = exp(-extinction * rayLength);
				dvec3 const sunTransmittance = sunTransmittanceTable.get(uvec2(layer + 1, a)) * transmittance;
				sunTransmittanceTable.set(uvec2(layer, a), sunTransmittance);
			}
		}
	}

	// std::cout << "Sun transmittance table:\n" << sunTransmittanceTable << '\n';
	return sunTransmittanceTable;
}

Scatterer::Scatterer(Atmosphere const &atmosphere, AtmosphereLayers const &layers)
:
	atmosphere(atmosphere),
	layers(layers),
	transmittanceTable(buildTransmittanceTable(atmosphere, layers)),
	sunTransmittanceTable(buildSunTransmittanceTable(atmosphere, layers))
{
}

dvec3 Scatterer::scatteredLight(dvec3 direction, dvec3 sunDirection, dvec3 sunColor) const {
	if (direction.z < 0.0) {
		return dvec3(0.0);
	}
	double const lightAngle = acos(dot(direction, sunDirection));
	double const groundAngle = acos(direction.z);
	// 0.0046 is the real angular radius of sun in radians
	dvec3 scatteredLight = (1.0 - smoothstep(0.04, 0.07, lightAngle)) * sunColor;
	for (int layer = layers.numLayers - 1; layer > 0; --layer) {
		double const lowerHeight = layers.heights[layer - 1];
		double const upperHeight = layers.heights[layer];
		double const lowerAngle = rayAngleAtHeight(0.0, lowerHeight, groundAngle, atmosphere.earthRadius);
		double const rayLength = rayLengthBetweenHeights(lowerHeight, upperHeight, lowerAngle, atmosphere.earthRadius);

		// Add inscattering, attenuated by optical depth to the sun
		dvec3 const rayleighInscattering =
			atmosphere.rayleighScattering.coefficient *
			atmosphere.rayleighScattering.phaseFunction(lightAngle) *
			sunTransmittanceTable(dvec2(layer, groundAngle));
		dvec3 const mieInscattering =
			atmosphere.mieScattering.coefficient *
			atmosphere.mieScattering.phaseFunction(lightAngle) *
			sunTransmittanceTable(dvec2(layer, groundAngle));
		scatteredLight += rayLength * sunColor * (rayleighInscattering + mieInscattering);

		// Multiply transmittance
		dvec3 const layerTransmittance = transmittanceTable(dvec2(layer, lowerAngle));
		scatteredLight *= layerTransmittance;
		/*
		std::cout
			<< layer << " -> "
			<< "[" << lowerHeight << ", " << upperHeight << "] "
			<< "opt " << rayleighOpticalLength << ' '
			<< "in " << rayleighInscattering.b << ' '
			<< "sunAtt " << sunAttenuation.b << ' '
			<< "layAtt " << layerAttenuation.b << ' '
			<< "now " << scatteredLightFactor
			<< '\n';
			*/
	}
	//std::cout << '\n';
	return scatteredLight;
}

Sky::Sky(Scatterer const &scatterer)
:
	scatterer(scatterer),
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

	glBindTexture(GL_TEXTURE_CUBE_MAP, texture.getName());
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	generateFaces();
}

dvec3 Sky::computeColor(vec3 dir) {
	// TODO get from sun
	dvec3 const direction(dir);
	dvec3 const sunColor = 1.0 * dvec3(1.0, 1.0, 1.0);
	dvec3 const sunDirection = normalize(dvec3(3.0, 0.0, 1.0));

	return scatterer.scatteredLight(direction, sunDirection, sunColor);

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
	glBindTexture(GL_TEXTURE_CUBE_MAP, texture.getName());
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

	glBindBuffer(GL_ARRAY_BUFFER, vertices.getName());
	glVertexPointer(3, GL_INT, 0, 0);
	glTexCoordPointer(3, GL_INT, 0, 0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, texture.getName());
	glDrawArrays(GL_QUADS, 0, vertices.getSizeInBytes() / sizeof(int) / 3);

	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisable(GL_TEXTURE_CUBE_MAP);
	glDepthMask(GL_TRUE);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
}
