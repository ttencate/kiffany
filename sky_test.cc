#include "sky.h"

#include <boost/test/unit_test.hpp>
#include <boost/bind.hpp>

#include <limits>

#define BOOST_LEVEL_COMP_3(level, comp, a, b) \
	BOOST_##level##_##comp(vec3(a).x, vec3(b).x); \
	BOOST_##level##_##comp(vec3(a).y, vec3(b).y); \
	BOOST_##level##_##comp(vec3(a).z, vec3(b).z)
#define BOOST_CHECK_LE_3(a, b) BOOST_LEVEL_COMP_3(CHECK, LE, a, b)
#define BOOST_CHECK_GE_3(a, b) BOOST_LEVEL_COMP_3(CHECK, GE, a, b)
#define BOOST_REQUIRE_LE_3(a, b) BOOST_LEVEL_COMP_3(REQUIRE, LE, a, b)
#define BOOST_REQUIRE_GE_3(a, b) BOOST_LEVEL_COMP_3(REQUIRE, GE, a, b)
#define BOOST_LEVEL_CLOSE_3(level, a, b, eps) \
	BOOST_##level##_CLOSE(vec3(a).x, vec3(b).x, eps); \
	BOOST_##level##_CLOSE(vec3(a).y, vec3(b).y, eps); \
	BOOST_##level##_CLOSE(vec3(a).z, vec3(b).z, eps)
#define BOOST_CHECK_CLOSE_3(a, b, eps) BOOST_LEVEL_CLOSE_3(CHECK, a, b, eps)
#define BOOST_REQUIRE_CLOSE_3(a, b, eps) BOOST_LEVEL_CLOSE_3(REQUIRE, a, b, eps)

namespace {
	struct Fixture {
		Atmosphere atmosphere;
		AtmosphereLayers layers;
		Fixture()
		:
			layers(atmosphere, 8)
		{
		}
	};
}

BOOST_FIXTURE_TEST_SUITE(SkyTest, Fixture)

float const EPS = 1e-4;

BOOST_AUTO_TEST_CASE(TestRayLengthUpwardsStraightUp) {
	BOOST_CHECK_CLOSE( 5.0, rayLengthUpwards(Ray( 0.0, 0.0),  5.0), EPS);
	BOOST_CHECK_CLOSE( 5.0, rayLengthUpwards(Ray( 0.0, 0.0),  5.0), EPS);
	BOOST_CHECK_CLOSE( 5.0, rayLengthUpwards(Ray( 0.0, 0.0),  5.0), EPS);
	BOOST_CHECK_CLOSE(10.0, rayLengthUpwards(Ray(50.0, 0.0), 60.0), EPS);
	BOOST_CHECK_CLOSE(10.0, rayLengthUpwards(Ray(50.0, 0.0), 60.0), EPS);
	BOOST_CHECK_CLOSE(10.0, rayLengthUpwards(Ray(50.0, 0.0), 60.0), EPS);
}

BOOST_AUTO_TEST_CASE(TestRayLengthUpwardsSideways) {
	BOOST_CHECK_CLOSE(50.0, rayLengthUpwards(Ray( 0.0, 0.5 * M_PI), 50.0), EPS);
	BOOST_CHECK_CLOSE(20.0, rayLengthUpwards(Ray( 0.0, 0.5 * M_PI), 20.0), EPS);
	BOOST_CHECK_CLOSE(40.0, rayLengthUpwards(Ray(30.0, 0.5 * M_PI), 50.0), EPS);
}

BOOST_AUTO_TEST_CASE(TestRayLengthDownwards) {
	BOOST_CHECK_EQUAL(40.0, rayLengthDownwards(Ray(50.0, M_PI), 10.0));
	BOOST_CHECK_EQUAL(20.0, rayLengthDownwards(Ray(50.0, M_PI), 30.0));
	BOOST_CHECK_EQUAL(10.0, rayLengthDownwards(Ray(20.0, M_PI), 10.0));
}

template<typename F>
void testPhaseFunctionIsValid(F phaseFunction) {
	unsigned const slices = 1024;
	float integral = 0;
	for (unsigned i = 0; i < slices; ++i) {
		float const startAngle = M_PI * i / slices;
		float const endAngle = M_PI * (i + 1) / slices;
		float const startValue = phaseFunction(startAngle);
		float const endValue = phaseFunction(endAngle);
		// The phase function should not be negative
		BOOST_REQUIRE_LE(0, startValue);
		BOOST_REQUIRE_LE(0, endValue);
		// Trapezoid integration
		float const meanValue = 0.5 * (startValue + endValue);
		float const area = 4.0 * M_PI * (sin(0.5 * endAngle) - sin(0.5 * startAngle));
		integral += meanValue * area;
	}
}

BOOST_AUTO_TEST_CASE(TestRayleighPhaseFunctionIsValid) {
	testPhaseFunctionIsValid(boost::bind(&RayleighScattering::phaseFunction, atmosphere.rayleighScattering, _1));
}

BOOST_AUTO_TEST_CASE(TestMiePhaseFunctionIsValid) {
	testPhaseFunctionIsValid(boost::bind(&MieScattering::phaseFunction, atmosphere.mieScattering, _1));
}

BOOST_AUTO_TEST_CASE(TestRayleighPhaseFunctionIsSymmetric) {
	float const EPS = 1e-6;
	for (unsigned a = 0; a < 16; ++a) {
		float const angle = 0.5 * M_PI * a / 16;
		BOOST_REQUIRE_CLOSE(
				atmosphere.rayleighScattering.phaseFunction(angle),
				atmosphere.rayleighScattering.phaseFunction(M_PI - angle), EPS);
	}
}

BOOST_AUTO_TEST_CASE(TestRayleighPhaseFunctionIsForward) {
	for (unsigned a = 1; a < 16; ++a) {
		float const angle = 0.5 * M_PI * a / 16;
		BOOST_REQUIRE_LT(
				atmosphere.rayleighScattering.phaseFunction(angle),
				atmosphere.rayleighScattering.phaseFunction(0.0));
	}
}

BOOST_AUTO_TEST_CASE(TestMiePhaseFunctionIsForward) {
	for (unsigned a = 1; a < 32; ++a) {
		float const angle = M_PI * a / 32;
		BOOST_REQUIRE_LT(
				atmosphere.mieScattering.phaseFunction(angle),
				atmosphere.mieScattering.phaseFunction(0.0));
	}
}

BOOST_AUTO_TEST_CASE(TestLayerHeights) {
	float const thickness = atmosphere.rayleighScattering.thickness;
	float const EPS = 1e-4;
	BOOST_CHECK_CLOSE(atmosphere.earthRadius, layers.heights[0], EPS);
	for (unsigned i = 0; i < layers.numLayers - 1; ++i) {
		BOOST_CHECK_LE(0.0, layers.heights[i]);
		BOOST_CHECK_LT(layers.heights[i], layers.heights[i + 1]);
		float cumulativeDensity = thickness * (1.0 - exp(-(layers.heights[i] - atmosphere.earthRadius) / thickness));
		BOOST_CHECK_CLOSE((float)i / (layers.numLayers - 1) * thickness, cumulativeDensity, EPS);
	}
	BOOST_CHECK_EQUAL(atmosphere.earthRadius + atmosphere.thickness, layers.heights[layers.numLayers - 1]);
}

BOOST_AUTO_TEST_CASE(TestBuildTransmittanceTable) {
	Vec3Table2D transmittanceTable = buildTransmittanceTable(atmosphere, layers);
	unsigned numLayers = layers.numLayers;
	unsigned numAngles = layers.numAngles;
	BOOST_REQUIRE_EQUAL(numLayers, transmittanceTable.getSize().x);
	BOOST_REQUIRE_EQUAL(numAngles, transmittanceTable.getSize().y);
	for (unsigned a = 0; a < numAngles; ++a) {
		for (unsigned layer = 0; layer < numLayers; ++layer) {
			uvec2 index = uvec2(layer, a);
			vec3 transmittance = transmittanceTable.get(index);
			// Transmittance is always between 0 and 1
			BOOST_REQUIRE_LE_3(0.0, transmittance);
			BOOST_REQUIRE_GE_3(1.0, transmittance);
			// Larger angles mean longer travel distance, hence less transmittance
			if (a > 0 && a < numAngles / 2) {
				BOOST_REQUIRE_LE_3(transmittance, transmittanceTable.get(uvec2(layer, a - 1)));
			}
		}
	}
}

BOOST_AUTO_TEST_CASE(TestBuildTransmittanceTableForZeroRadiusEarth) {
	Atmosphere atmosphere(0.0);
	Vec3Table2D transmittanceTable = buildTransmittanceTable(atmosphere, layers);
	unsigned numLayers = layers.numLayers;
	unsigned numAngles = layers.numAngles;
	BOOST_REQUIRE_EQUAL(numLayers, transmittanceTable.getSize().x);
	BOOST_REQUIRE_EQUAL(numAngles, transmittanceTable.getSize().y);
	for (unsigned a = 0; a < numAngles; ++a) {
		for (unsigned layer = 0; layer < numLayers; ++layer) {
			uvec2 index = uvec2(layer, a);
			vec3 transmittance = transmittanceTable.get(index);
			// Transmittance is always between 0 and 1
			BOOST_REQUIRE_LE_3(0.0, transmittance);
			BOOST_REQUIRE_GE_3(1.0, transmittance);
			// For an earth of radius 0, all ground angles should give the same result
			if (layer == 0 && a < numAngles / 2) {
				BOOST_REQUIRE_CLOSE_3(transmittanceTable.get(uvec2(layer, 0)), transmittanceTable.get(uvec2(layer, a)), EPS);
			}
			// Larger angles mean longer travel distance, hence less transmittance
			if (a > 0 && a < numAngles / 2) {
				BOOST_REQUIRE_LE_3(transmittance, transmittanceTable.get(uvec2(layer, a - 1)));
			}
		}
	}
}

BOOST_AUTO_TEST_CASE(TestBuildTotalTransmittanceTable) {
	Vec3Table2D transmittanceTable = buildTransmittanceTable(atmosphere, layers);
	Vec3Table2D totalTransmittanceTable = buildTotalTransmittanceTable(atmosphere, layers, transmittanceTable);
	unsigned numLayers = layers.numLayers;
	unsigned numAngles = layers.numAngles;
	BOOST_REQUIRE_EQUAL(numLayers, totalTransmittanceTable.getSize().x);
	BOOST_REQUIRE_EQUAL(numAngles, totalTransmittanceTable.getSize().y);
	for (unsigned a = 0; a < numAngles; ++a) {
		for (unsigned layer = 0; layer < numLayers; ++layer) {
			uvec2 index = uvec2(layer, a);
			vec3 totalTransmittance = totalTransmittanceTable.get(index);
			// Transmittance is always between 0 and 1
			BOOST_REQUIRE_LE_3(0.0, totalTransmittance);
			BOOST_REQUIRE_GE_3(1.0, totalTransmittance);
			// Larger angles mean longer travel distance, hence less transmittance
			if (a > 0) {
				BOOST_REQUIRE_LE_3(totalTransmittance, totalTransmittanceTable.get(uvec2(layer, a - 1)));
			}
			// Higher layers mean shorter travel distance, hence smaller optical length
			if (layer > 0) {
				BOOST_REQUIRE_GE_3(totalTransmittance, totalTransmittanceTable.get(uvec2(layer - 1, a)));
			}
		}
	}
}

BOOST_AUTO_TEST_CASE(TestBuildTotalTransmittanceTableForZeroRadiusEarth) {
	Atmosphere atmosphere(0.0);
	Vec3Table2D transmittanceTable = buildTransmittanceTable(atmosphere, layers);
	Vec3Table2D totalTransmittanceTable = buildTotalTransmittanceTable(atmosphere, layers, transmittanceTable);
	unsigned numLayers = layers.numLayers;
	unsigned numAngles = layers.numAngles;
	BOOST_REQUIRE_EQUAL(numLayers, totalTransmittanceTable.getSize().x);
	BOOST_REQUIRE_EQUAL(numAngles, totalTransmittanceTable.getSize().y);
	for (unsigned a = 0; a < numAngles; ++a) {
		for (unsigned layer = 0; layer < numLayers; ++layer) {
			uvec2 index = uvec2(layer, a);
			vec3 totalTransmittance = totalTransmittanceTable.get(index);
			// Transmittance is always between 0 and 1
			BOOST_REQUIRE_LE_3(0.0, totalTransmittance);
			BOOST_REQUIRE_GE_3(1.0, totalTransmittance);
			// For an earth of radius 0, all ground angles should give the same result
			if (layer == 0 && a < numAngles / 2) {
				BOOST_REQUIRE_CLOSE_3(totalTransmittanceTable.get(uvec2(layer, 0)), totalTransmittanceTable.get(uvec2(layer, a)), EPS);
			}
			// Higher layers mean shorter travel distance, hence more transmittance,
			// unless we were hitting the earth previously
			if (layer > 0 && a < numAngles / 2) {
				BOOST_REQUIRE_GE_3(totalTransmittance, totalTransmittanceTable.get(uvec2(layer - 1, a)));
			}
		}
	}
}

BOOST_AUTO_TEST_CASE(TestScatterer) {
	Scatterer scatterer(atmosphere, layers);
	Sun sun(0.5f, 0.0f, 0.0f, 0.0f, 0.0f, vec3(1.0f), 0.5f);

	vec3 intoSunFactor = scatterer.scatteredLight(vec3(0.0, 0.0, 1.0), sun);
	BOOST_CHECK_LE_3(0.0, intoSunFactor);
	BOOST_CHECK_GE_3(1.0, intoSunFactor);

	vec3 nextToSunFactor = scatterer.scatteredLight(normalize(vec3(1.0, 0.0, 1.0)), sun);
	BOOST_CHECK_LE_3(0.0, nextToSunFactor);
	BOOST_CHECK_GE_3(1.0, nextToSunFactor);

	BOOST_CHECK_GE_3(intoSunFactor, nextToSunFactor);
	BOOST_CHECK_LE(nextToSunFactor.r, nextToSunFactor.g);
	BOOST_CHECK_LE(nextToSunFactor.g, nextToSunFactor.b);

	vec3 closeToHorizonFactor = scatterer.scatteredLight(normalize(vec3(100.0, 0.0, 1.0)), sun);
	BOOST_CHECK_LE_3(0.0, closeToHorizonFactor);
	BOOST_CHECK_GE_3(1.0, closeToHorizonFactor);
}

BOOST_AUTO_TEST_SUITE_END()
