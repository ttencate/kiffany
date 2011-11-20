#include "sky.h"

#include <algorithm>
#include <limits>

/* References:
 * Preetham, Shirley, Smits, "A practical model for daylight",
 *     http://www.cs.utah.edu/~shirley/papers/sunsky/sunsky.pdf
 * Nishita, Sirai, Tadamura, Nakamae, "Display of the Earth taking into account atmospheric scattering",
 *     http://citeseerx.ist.psu.edu/viewdoc/download?doi=10.1.1.145.5229&rep=rep1&type=pdf
 * http://developer.amd.com/media/gpu_assets/ATI-LightScattering.pdf
 * http://hal.archives-ouvertes.fr/docs/00/28/87/58/PDF/article.pdf !!!
 * http://developer.amd.com/media/gpu_assets/PreethamSig2003CourseNotes.pdf
 */

dvec3 const Atmosphere::lambda = dvec3(680e-9, 550e-9, 440e-9); // wavelengths of RGB (metres)

dvec3 Atmosphere::computeRayleighScatteringCoefficient() const {
	double const n = 1.000293; // index of refraction of air
	double const Ns = 2.5e25; // number density in standard atmosphere (molecules/m^3)
	double const K = 2.0 * M_PI * M_PI * sqr(sqr(n) - 1.0) / (3.0 * Ns);
	return
		4.0 * M_PI * K
		/ pow(lambda, dvec3(4.0));
}

dvec3 Atmosphere::computeMieScatteringCoefficient() const {
	// TODO why do we need a much smaller number than in the literature?
	double const c = 6e-20; // 6e-17 for clear, 25e-17 for overcast
	double const nu = 4.0;
	dvec3 K(0.68, 0.673, 0.663); // Preetham, Table 2
	return 0.434 * c * M_PI * pow(2 * M_PI / lambda, dvec3(nu - 2.0)) * K;
}

Atmosphere::Atmosphere()
:
	earthRadius(6371e3),
	rayleighHeight(7994),
	mieHeight(1200),
	atmosphereHeight(100e3),
	rayleighScatteringCoefficient(computeRayleighScatteringCoefficient()),
	mieScatteringCoefficient(computeMieScatteringCoefficient())
{
}

double Atmosphere::rayleighDensityAtHeight(double height) const {
	return exp(-height / rayleighHeight);
}

double Atmosphere::mieDensityAtHeight(double height) const {
	return exp(-height / mieHeight);
}

double Atmosphere::rayAngleAtHeight(double angle, double height) const {
	return asin(earthRadius * sin(angle) / (earthRadius + height));
}

// TODO handle downward angles correctly
double Atmosphere::rayLengthBetweenHeights(double angle, double lowerHeight, double upperHeight) const {
	double cosAngle = cos(angle);
	return /*+-*/sqrt(
			sqr(earthRadius + lowerHeight) * (sqr(cosAngle) - 1.0) +
			sqr(earthRadius + upperHeight)) -
		(earthRadius + lowerHeight) * cosAngle;
}

double Atmosphere::rayLengthToHeight(double angle, double height) const {
	return rayLengthBetweenHeights(angle, 0, height);
}

double Atmosphere::rayleighPhaseFunction(double angle) const {
	return
		3.0 / (16.0 * M_PI)
		* (1.0 + sqr(cos(angle)));
}

double Atmosphere::miePhaseFunction(double angle) const {
	/*
	double const u = 0.75; // Mie scattering parameter (0.7-0.85)
	double const x = 5.0/9.0 * u + 125.0/729.0 * u*u*u + sqrt(64.0/27.0 - 325.0/243.0 * u*u + 1250.0/2187.0 * u*u*u*u);
	double const g = 5.0/9.0 * u - (4.0/3.0 - 25.0/81.0 * u*u) * pow(x, -1.0/3.0) + pow(x, 1.0/3.0);
	double const c = cos(angle);
	return 3.0 * (1.0 - sqr(g)) * (1.0 + sqr(c)) /
		(2.0 * (2.0 + g*g) * pow(1.0 + g*g - 2.0 * g * c, 3.0/2.0));
		*/
	double const u = 0.85; // Mie scattering parameter (0.7-0.85)
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
		(4.0 * M_PI * pow(1 - 2.0 * g * cos(angle) + sqr(g), 3.0/2.0));
}

dvec3 Atmosphere::rayleighScatteringAtHeight(double angle, double height) const {
	return rayleighScatteringCoefficient * rayleighDensityAtHeight(height) * rayleighPhaseFunction(angle);
}

dvec3 Atmosphere::mieScatteringAtHeight(double angle, double height) const {
	return mieScatteringCoefficient * mieDensityAtHeight(height) * miePhaseFunction(angle);
}

dvec3 Atmosphere::attenuationFromOpticalLength(dvec3 opticalLength) const {
	return exp(-opticalLength);
}

AtmosphereLayers::Heights AtmosphereLayers::computeHeights(double rayleighHeight, double atmosphereHeight) {
	Heights heights(numLayers);
	for (unsigned i = 0; i < numLayers - 1; ++i) {
		double const cumulativeDensity = 1.0f - (double)i / numLayers;
		heights[i] = -rayleighHeight * log(cumulativeDensity);
	}
	heights[numLayers - 1] = std::max(heights[numLayers - 1] * 1.01f, atmosphereHeight);
	return heights;
}

AtmosphereLayers::AtmosphereLayers(Atmosphere const &atmosphere)
:
	// TODO why is the number of layers so influential on the end result?
	numLayers(16),
	numAngles(32),
	heights(computeHeights(atmosphere.rayleighHeight, atmosphere.atmosphereHeight))
{
}

Dvec3Table2D buildOpticalLengthTable(Atmosphere const &atmosphere, AtmosphereLayers const &layers) {
	unsigned const numAngles = layers.numAngles;
	unsigned const numLayers = layers.numLayers;
	dvec3 const rayleighScatteringCoefficient = atmosphere.rayleighScatteringCoefficient;
	dvec3 const mieScatteringCoefficient = atmosphere.mieScatteringCoefficient;

	Dvec3Table2D opticalLengthTable = Dvec3Table2D::createWithCoordsSizeAndOffset(
			uvec2(numLayers, numAngles),
			dvec2(numLayers, M_PI * numAngles / (numAngles - 1)),
			dvec2(0.0, 0.0));
	for (unsigned a = 0; a < numAngles; ++a) {
		double const angle = M_PI * a / (numAngles - 1);
		if (angle >= M_PI / 2) {
			// Ray goes into the ground
			for (unsigned layer = 0; layer < numLayers; ++layer) {
				opticalLengthTable.set(uvec2(layer, a), opticalLengthTable.get(uvec2(layer, a - 1)));
			}
		} else {
			// Compute optical length between this layer and next
			for (unsigned layer = 0; layer < numLayers - 1; ++layer) {
				double const lowerHeight = layers.heights[layer];
				double const upperHeight = layers.heights[layer + 1];
				double const length =
					atmosphere.rayLengthToHeight(angle, upperHeight) -
					atmosphere.rayLengthToHeight(angle, lowerHeight);
				double const airMass = length * 0.5 * (
						atmosphere.rayleighDensityAtHeight(lowerHeight) +
						atmosphere.rayleighDensityAtHeight(upperHeight));
				double const aerosolMass = length * 0.5 * (
						atmosphere.mieDensityAtHeight(lowerHeight) +
						atmosphere.mieDensityAtHeight(upperHeight));
				dvec3 const segment =
					rayleighScatteringCoefficient * airMass +
					mieScatteringCoefficient * aerosolMass;
				opticalLengthTable.set(uvec2(layer, a), segment);
			}
			opticalLengthTable.set(uvec2(numLayers - 1, a), dvec3(0.0));
		}
	}

	return opticalLengthTable;
}

Dvec3Table2D buildOpticalDepthTable(Atmosphere const &atmosphere, AtmosphereLayers const &layers) {
	unsigned const numAngles = layers.numAngles;
	unsigned const numLayers = layers.numLayers;
	dvec3 const rayleighScatteringCoefficient = atmosphere.rayleighScatteringCoefficient;
	dvec3 const mieScatteringCoefficient = atmosphere.mieScatteringCoefficient;

	Dvec3Table2D opticalDepthTable = Dvec3Table2D::createWithCoordsSizeAndOffset(
			uvec2(numLayers, numAngles),
			dvec2(numLayers, M_PI * numAngles / (numAngles - 1)),
			dvec2(0.0, 0.0));
	for (unsigned a = 0; a < numAngles; ++a) {
		double const angle = M_PI * a / (numAngles - 1);
		if (angle >= M_PI / 2) {
			// Ray goes into the ground
			for (unsigned layer = 0; layer < numLayers; ++layer) {
				opticalDepthTable.set(uvec2(layer, a), opticalDepthTable.get(uvec2(layer, a - 1)));
			}
		} else {
			// Cumulatively sum over all layers
			opticalDepthTable.set(uvec2(numLayers - 1, a), dvec3(0.0));
			for (int layer = numLayers - 2; layer >= 0; --layer) {
				double const lowerHeight = layers.heights[layer];
				double const upperHeight = layers.heights[layer + 1];
				double const length =
					atmosphere.rayLengthToHeight(angle, upperHeight) -
					atmosphere.rayLengthToHeight(angle, lowerHeight);
				double const airMass = length * 0.5 * (
						atmosphere.rayleighDensityAtHeight(lowerHeight) +
						atmosphere.rayleighDensityAtHeight(upperHeight));
				double const aerosolMass = length * 0.5 * (
						atmosphere.mieDensityAtHeight(lowerHeight) +
						atmosphere.mieDensityAtHeight(upperHeight));
				dvec3 const segment =
					rayleighScatteringCoefficient * airMass +
					mieScatteringCoefficient * aerosolMass;
				dvec3 const opticalDepth = opticalDepthTable.get(uvec2(layer + 1, a)) + segment;
				opticalDepthTable.set(uvec2(layer, a), opticalDepth);
			}
		}
	}

	return opticalDepthTable;
}

Dvec3Table2D buildSunAttenuationTable(Atmosphere const &atmosphere, AtmosphereLayers const &layers, Dvec3Table2D const &opticalLengthTable) {
	unsigned const numLayers = layers.numLayers;
	unsigned const numAngles = layers.numAngles;

	// Convert to attenuation factor
	Dvec3Table2D sunAttenuationTable = Dvec3Table2D::createWithCoordsSizeAndOffset(
			uvec2(numLayers, numAngles),
			dvec2(numLayers, M_PI * numAngles / (numAngles - 1)),
			dvec2(0.0, 0.0));
	for (unsigned a = 0; a < numAngles; ++a) {
		for (unsigned layer = 0; layer < numLayers; ++layer) {
			uvec2 index(layer, a);
			dvec3 const opticalLength = opticalLengthTable.get(index);
			dvec3 const sunAttenuation = atmosphere.attenuationFromOpticalLength(opticalLength);
			sunAttenuationTable.set(index, sunAttenuation);
		}
	}

	return sunAttenuationTable;
}

Scatterer::Scatterer(Atmosphere const &atmosphere, AtmosphereLayers const &layers)
:
	atmosphere(atmosphere),
	layers(layers),
	opticalLengthTable(buildOpticalLengthTable(atmosphere, layers)),
	sunAttenuationTable(buildSunAttenuationTable(atmosphere, layers, buildOpticalDepthTable(atmosphere, layers)))
{
}

dvec3 Scatterer::scatteredLightFactor(dvec3 direction, dvec3 sunDirection) const {
	dvec3 scatteredLightFactor(0.0);
	double const lightAngle = acos(dot(direction, sunDirection));
	double const groundAngle = acos(direction.z);
	for (int layer = layers.numLayers - 1; layer >= 0; --layer) {
		double const height = layers.heights[layer];
		scatteredLightFactor +=
			sunAttenuationTable(dvec2(layer, groundAngle)) * (
					atmosphere.rayleighScatteringAtHeight(lightAngle, height) +
					atmosphere.mieScatteringAtHeight(lightAngle, height));
		double angle = atmosphere.rayAngleAtHeight(groundAngle, height);
		scatteredLightFactor *=
			exp(-opticalLengthTable(dvec2(layer, angle)));
	}
	return scatteredLightFactor;
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
	dvec3 const sunColor = 5.0e4 * dvec3(1.0, 1.0, 1.0);
	dvec3 const sunDirection = normalize(dvec3(20.0, 0.0, 1.0));

	dvec3 color = sunColor * scatterer.scatteredLightFactor(direction, sunDirection);
	return color;
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
