#include "sky.h"

#include <algorithm>
#include <limits>

void Atmosphere::updateLayerHeights(unsigned numLayers) {
	layerHeights.resize(numLayers);
	for (unsigned i = 0; i < numLayers - 1; ++i) {
		double const rho = 1.0f - (double)i / numLayers;
		layerHeights[i] = -rayleighHeight * log(rho);
	}
	layerHeights[numLayers - 1] = std::max(layerHeights[numLayers] * 1.01f, atmosphereHeight);
}

Atmosphere::Atmosphere()
:
	earthRadius(6371e3),
	rayleighHeight(7994),
	mieHeight(1200),
	atmosphereHeight(100e3),
	numAngles(32)
{
	dvec3 const lambda = dvec3(642e-9, 508e-9, 436e-9); // wavelengths of RGB (metres)
	double const n = 1.000293; // index of refraction of air
	double const Ns = 2.5e25; // number density in standard atmosphere (molecules/m^3)
	double const K = 2.0f * M_PI * M_PI * (n * n - 1.0f) * (n * n - 1.0f) / (3.0f * Ns);
	rayleighAttenuationFactor = 4.0 * M_PI * K / pow(lambda, dvec3(4.0)); // extinction ratio per unit of optical length

	updateLayerHeights(8);
}

double Atmosphere::rayleighDensityAtHeight(double height) const {
	return exp(-height / rayleighHeight);
}

double Atmosphere::rayleighDensityAtLayer(unsigned layer) const {
	return rayleighDensityAtHeight(layerHeights[layer]);
}

double Atmosphere::rayLengthToHeight(double angle, double height) const {
	return sqrt(-sqr(sin(angle) * earthRadius) + sqr(earthRadius + height));
}

double Atmosphere::rayLengthToLayer(double angle, unsigned layer) const {
	return rayLengthToHeight(angle, layerHeights[layer]);
}

dvec3 Atmosphere::attenuationFromOpticalLength(double opticalLength) const {
	return exp(-rayleighAttenuationFactor * opticalLength);
}

DoubleTable2D buildOpticalLengthTable(Atmosphere const &atmosphere) {
	unsigned const numAngles = atmosphere.getNumAngles();
	unsigned const numLayers = atmosphere.getNumLayers();

	DoubleTable2D opticalLengthTable = DoubleTable2D::createWithCoordsSizeAndOffset(
			uvec2(numLayers, numAngles),
			dvec2(numLayers, M_PI * numAngles / (numAngles - 1)),
			dvec2(0.0, 0.0));
	for (unsigned a = 0; a < numAngles; ++a) {
		double const angle = M_PI * a / (numAngles - 1);
		if (angle >= M_PI / 2) {
			// Ray goes into the ground
			for (unsigned layer = 0; layer <= numLayers; ++layer) {
				opticalLengthTable.set(uvec2(layer, a), std::numeric_limits<double>::infinity());
			}
		} else {
			// Cumulatively integrate over all layers
			opticalLengthTable.set(uvec2(numLayers - 1, a), 0);
			for (int layer = numLayers - 2; layer >= 0; --layer) {
				double const length =
					atmosphere.rayLengthToLayer(angle, layer + 1) -
					atmosphere.rayLengthToLayer(angle, layer);
				double const segment = length * 0.5 * (
						atmosphere.rayleighDensityAtLayer(layer + 1) +
						atmosphere.rayleighDensityAtLayer(layer));
				double const sum = opticalLengthTable.get(uvec2(layer + 1, a)) + segment;
				opticalLengthTable.set(uvec2(layer, a), sum);
			}
		}
	}

	return opticalLengthTable;
}

Dvec3Table2D buildSunAttenuationTable(Atmosphere const &atmosphere, DoubleTable2D const &opticalLengthTable) {
	unsigned const numAngles = atmosphere.getNumAngles();
	unsigned const numLayers = atmosphere.getNumLayers();

	// Convert to attenuation factor
	Dvec3Table2D sunAttenuationTable = Dvec3Table2D::createWithCoordsSizeAndOffset(
			uvec2(numLayers, numAngles),
			dvec2(numLayers, M_PI * numAngles / (numAngles - 1)),
			dvec2(0.0, 0.0));
	for (unsigned a = 0; a < numAngles; ++a) {
		for (unsigned layer = 0; layer < numLayers; ++layer) {
			uvec2 index(a, layer);
			double const opticalLength = opticalLengthTable.get(index);
			dvec3 const sunAttenuation = atmosphere.attenuationFromOpticalLength(opticalLength);
			sunAttenuationTable.set(index, sunAttenuation);
		}
	}

	return sunAttenuationTable;
}

Sky::Sky(Atmosphere const &atmosphere)
:
	atmosphere(atmosphere),
	size(16),
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
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	generateFaces();
}

vec3 Sky::computeColor(vec3 dir) {
	// TODO get from sun
	dvec3 const direction(dir);
	dvec3 const sunColor = dvec3(1.0, 1.0, 1.0);
	dvec3 const sunDirection = normalize(dvec3(1.0, 0.0, 1.0));
	
	double const cosPhi = dot(direction, sunDirection);
	dvec3 color(0, 0, 0);

	// TODO make static
	// Rayleigh parameters
	dvec3 const lambda = dvec3(642e-9, 508e-9, 436e-9); // wavelengths of RGB (metres)
	dvec3 const lambda4 = lambda * lambda * lambda * lambda;
	double const n = 1.000293; // index of refraction of air
	double const Ns = 2.5e25; // number density in standard atmosphere (molecules/m^3)
	double const K = 2.0f * M_PI * M_PI * (n * n - 1.0f) * (n * n - 1.0f) / (3.0f * Ns);
	dvec3 const KR = K / lambda4;
	double const FR = 0.75 * (1.0 + sqr(cosPhi));
	//std::cerr << "KR = " << glm::to_string(KR) << '\n';

	// Mie parameters
	double const u = 0.75; // Mie scattering parameter (0.7-0.85)
	double const x = 5.0/9.0 * u + 125.0/729.0 * u*u*u + sqrt(64.0/27.0 - 325.0/243.0 * u*u + 1250.0/2187.0 * u*u*u*u);
	double const g = 5.0/9.0 * u - (4.0/3.0 - 25.0/81.0 * u*u) * pow(x, -1.0/3.0) + pow(x, 1.0/3.0);
	double const FM = 3.0 * (1.0 - g*g) * (1.0 + cosPhi*cosPhi) /(2.0 * (2.0 + g*g) * pow(1.0 + g*g - 2.0 * g * cosPhi, 3.0/2.0));

	LayerHeights const &layerHeighths = atmosphere.getLayerHeights();
	for (unsigned i = atmosphere.getNumLayers() - 1; i >= 1; --i) {
		/*
		double const bottomHeight = layerHeights[i - 1];
		double const topHeight = layerHeights[i];
		double const middleHeight = 0.5f * (bottomHeight + topHeight);
		double const length =
			sqrt(sqr(earthRadius) * (sqr(direction.z) - 1.0) + sqr(earthRadius + topHeight)) -
			sqrt(sqr(earthRadius) * (sqr(direction.z) - 1.0) + sqr(earthRadius + bottomHeight));

		double const rhoR = expf(-middleHeight / rayleighHeight);
		double const rhoM = expf(-middleHeight / mieHeight);

		// scatter
		//std::cerr << "Length times rho " << (length * rhoR) << '\n';
		//std::cerr << "Scatter " << glm::to_string(length * KR * rhoR * FR * sunColor) << '\n';
		//std::cerr << "KR " << glm::to_string(KR) << " rhoR " << rhoR << " FR " << FR << '\n';
		color += length * KR * rhoR * FR * sunColor; // Rayleigh
		//color += length *      rhoM * FM * sunColor; // Mie
		//std::cerr << "Color now " << glm::to_string(color) << '\n';

		// attenuate
		//std::cerr << "Attenuate " << glm::to_string(gv) << '\n';
		color *= exp(-length * KR * rhoR); // Rayleigh
		//color *= exp(-length *      rhoM); // Mie
		*/
	}
	//std::cerr << '\n';
	return vec3(color);
}

void Sky::generateFace(GLenum face, vec3 base, vec3 xBasis, vec3 yBasis) {
	unsigned char *p = textureImage.get();
	for (unsigned y = 0; y < size; ++y) {
		for (unsigned x = 0; x < size; ++x) {
			vec3 direction = normalize(
				base +
				(x + 0.5f) / size * xBasis +
				(y + 0.5f) / size * yBasis);
			vec3 color = computeColor(direction);
			color = clamp(color, 0.0f, 1.0f);
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
