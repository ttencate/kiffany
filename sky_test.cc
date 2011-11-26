#include "sky.h"

#include <boost/test/unit_test.hpp>
#include <boost/bind.hpp>

#include <limits>

#define BOOST_LEVEL_COMP_3(level, comp, a, b) \
	BOOST_##level##_##comp(dvec3(a).x, dvec3(b).x); \
	BOOST_##level##_##comp(dvec3(a).y, dvec3(b).y); \
	BOOST_##level##_##comp(dvec3(a).z, dvec3(b).z)
#define BOOST_CHECK_LE_3(a, b) BOOST_LEVEL_COMP_3(CHECK, LE, a, b)
#define BOOST_CHECK_GE_3(a, b) BOOST_LEVEL_COMP_3(CHECK, GE, a, b)
#define BOOST_REQUIRE_LE_3(a, b) BOOST_LEVEL_COMP_3(REQUIRE, LE, a, b)
#define BOOST_REQUIRE_GE_3(a, b) BOOST_LEVEL_COMP_3(REQUIRE, GE, a, b)
#define BOOST_LEVEL_CLOSE_3(level, a, b, eps) \
	BOOST_##level##_CLOSE(dvec3(a).x, dvec3(b).x, eps); \
	BOOST_##level##_CLOSE(dvec3(a).y, dvec3(b).y, eps); \
	BOOST_##level##_CLOSE(dvec3(a).z, dvec3(b).z, eps)
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

double const EPS = 1e-4;

BOOST_AUTO_TEST_CASE(TestRayLengthUpwardsStraightUp) {
	BOOST_CHECK_CLOSE(5.0, rayLengthUpwards(0.0, 5.0, 0.0, 0.0), EPS);
	BOOST_CHECK_CLOSE(5.0, rayLengthUpwards(0.0, 5.0, 0.0, 1.0), EPS);
	BOOST_CHECK_CLOSE(5.0, rayLengthUpwards(0.0, 5.0, 0.0, 100.0), EPS);
	BOOST_CHECK_CLOSE(10.0, rayLengthUpwards(50.0, 60.0, 0.0, 0.0), EPS);
	BOOST_CHECK_CLOSE(10.0, rayLengthUpwards(50.0, 60.0, 0.0, 1.0), EPS);
	BOOST_CHECK_CLOSE(10.0, rayLengthUpwards(50.0, 60.0, 0.0, 100.0), EPS);
}

BOOST_AUTO_TEST_CASE(TestRayLengthUpwardsSideways) {
	BOOST_CHECK_CLOSE(50.0, rayLengthUpwards(0.0, 50.0, 0.5 * M_PI, 0.0), EPS);
	BOOST_CHECK_CLOSE(40.0, rayLengthUpwards(30.0, 50.0, 0.5 * M_PI, 0.0), EPS);
	BOOST_CHECK_CLOSE(40.0, rayLengthUpwards(0.0, 20.0, 0.5 * M_PI, 30.0), EPS);
}

BOOST_AUTO_TEST_CASE(TestRayLengthDownwards) {
	BOOST_CHECK_EQUAL(50.0, rayLengthDownwards(50.0, 0.0, M_PI, 30.0));
	BOOST_CHECK_EQUAL(20.0, rayLengthDownwards(50.0, 30.0, M_PI, 0.0));
	BOOST_CHECK_EQUAL(20.0, rayLengthDownwards(20.0, 0.0, M_PI, 30.0));
}

template<typename F>
void testPhaseFunctionIsValid(F phaseFunction) {
	unsigned const slices = 1024;
	double integral = 0;
	for (unsigned i = 0; i < slices; ++i) {
		double const startAngle = M_PI * i / slices;
		double const endAngle = M_PI * (i + 1) / slices;
		double const startValue = phaseFunction(startAngle);
		double const endValue = phaseFunction(endAngle);
		// The phase function should not be negative
		BOOST_REQUIRE_LE(0, startValue);
		BOOST_REQUIRE_LE(0, endValue);
		// Trapezoid integration
		double const meanValue = 0.5 * (startValue + endValue);
		double const area = 4.0 * M_PI * (sin(0.5 * endAngle) - sin(0.5 * startAngle));
		integral += meanValue * area;
	}
	// The phase function integrated over a sphere should be 1
	// TODO figure out if this is true
	// double const EPS = 1e0;
	// BOOST_CHECK_CLOSE(1.0, integral, EPS);
}

BOOST_AUTO_TEST_CASE(TestRayleighPhaseFunctionIsValid) {
	testPhaseFunctionIsValid(boost::bind(&RayleighScattering::phaseFunction, atmosphere.rayleighScattering, _1));
}

BOOST_AUTO_TEST_CASE(TestMiePhaseFunctionIsValid) {
	testPhaseFunctionIsValid(boost::bind(&MieScattering::phaseFunction, atmosphere.mieScattering, _1));
}

BOOST_AUTO_TEST_CASE(TestRayleighPhaseFunctionIsSymmetric) {
	double const EPS = 1e-6;
	for (unsigned a = 0; a < 16; ++a) {
		double const angle = 0.5 * M_PI * a / 16;
		BOOST_REQUIRE_CLOSE(
				atmosphere.rayleighScattering.phaseFunction(angle),
				atmosphere.rayleighScattering.phaseFunction(M_PI - angle), EPS);
	}
}

BOOST_AUTO_TEST_CASE(TestRayleighPhaseFunctionIsForward) {
	for (unsigned a = 1; a < 16; ++a) {
		double const angle = 0.5 * M_PI * a / 16;
		BOOST_REQUIRE_LT(
				atmosphere.rayleighScattering.phaseFunction(angle),
				atmosphere.rayleighScattering.phaseFunction(0.0));
	}
}

BOOST_AUTO_TEST_CASE(TestMiePhaseFunctionIsForward) {
	for (unsigned a = 1; a < 32; ++a) {
		double const angle = M_PI * a / 32;
		BOOST_REQUIRE_LT(
				atmosphere.mieScattering.phaseFunction(angle),
				atmosphere.mieScattering.phaseFunction(0.0));
	}
}

BOOST_AUTO_TEST_CASE(TestLayerHeights) {
	double const height = atmosphere.rayleighScattering.height;
	double const EPS = 1e-4;
	BOOST_CHECK_EQUAL(0.0, layers.heights[0]);
	for (unsigned i = 0; i < layers.numLayers - 1; ++i) {
		BOOST_CHECK_LE(0.0, layers.heights[i]);
		BOOST_CHECK_LT(layers.heights[i], layers.heights[i + 1]);
		double cumulativeDensity = height * (1.0 - exp(-layers.heights[i] / height));
		BOOST_CHECK_CLOSE((double)i / (layers.numLayers - 1) * height, cumulativeDensity, EPS);
	}
	BOOST_CHECK_EQUAL(atmosphere.atmosphereHeight, layers.heights[layers.numLayers - 1]);
}

BOOST_AUTO_TEST_CASE(TestBuildTransmittanceTable) {
	Dvec3Table2D transmittanceTable = buildTransmittanceTable(atmosphere, layers);
	unsigned numLayers = layers.numLayers;
	unsigned numAngles = layers.numAngles;
	BOOST_REQUIRE_EQUAL(numLayers, transmittanceTable.getSize().x);
	BOOST_REQUIRE_EQUAL(numAngles, transmittanceTable.getSize().y);
	for (unsigned a = 0; a < numAngles; ++a) {
		for (unsigned layer = 0; layer < numLayers; ++layer) {
			uvec2 index = uvec2(layer, a);
			dvec3 transmittance = transmittanceTable.get(index);
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
	Dvec3Table2D transmittanceTable = buildTransmittanceTable(atmosphere, layers);
	unsigned numLayers = layers.numLayers;
	unsigned numAngles = layers.numAngles;
	BOOST_REQUIRE_EQUAL(numLayers, transmittanceTable.getSize().x);
	BOOST_REQUIRE_EQUAL(numAngles, transmittanceTable.getSize().y);
	for (unsigned a = 0; a < numAngles; ++a) {
		for (unsigned layer = 0; layer < numLayers; ++layer) {
			uvec2 index = uvec2(layer, a);
			dvec3 transmittance = transmittanceTable.get(index);
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
	Dvec3Table2D transmittanceTable = buildTransmittanceTable(atmosphere, layers);
	Dvec3Table2D totalTransmittanceTable = buildTotalTransmittanceTable(atmosphere, layers, transmittanceTable);
	unsigned numLayers = layers.numLayers;
	unsigned numAngles = layers.numAngles;
	BOOST_REQUIRE_EQUAL(numLayers, totalTransmittanceTable.getSize().x);
	BOOST_REQUIRE_EQUAL(numAngles, totalTransmittanceTable.getSize().y);
	for (unsigned a = 0; a < numAngles; ++a) {
		for (unsigned layer = 0; layer < numLayers; ++layer) {
			uvec2 index = uvec2(layer, a);
			dvec3 totalTransmittance = totalTransmittanceTable.get(index);
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
	Dvec3Table2D transmittanceTable = buildTransmittanceTable(atmosphere, layers);
	Dvec3Table2D totalTransmittanceTable = buildTotalTransmittanceTable(atmosphere, layers, transmittanceTable);
	unsigned numLayers = layers.numLayers;
	unsigned numAngles = layers.numAngles;
	BOOST_REQUIRE_EQUAL(numLayers, totalTransmittanceTable.getSize().x);
	BOOST_REQUIRE_EQUAL(numAngles, totalTransmittanceTable.getSize().y);
	for (unsigned a = 0; a < numAngles; ++a) {
		for (unsigned layer = 0; layer < numLayers; ++layer) {
			uvec2 index = uvec2(layer, a);
			dvec3 totalTransmittance = totalTransmittanceTable.get(index);
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

	dvec3 intoSunFactor = scatterer.scatteredLight(dvec3(0.0, 0.0, 1.0), sun);
	BOOST_CHECK_LE_3(0.0, intoSunFactor);
	BOOST_CHECK_GE_3(1.0, intoSunFactor);

	dvec3 nextToSunFactor = scatterer.scatteredLight(normalize(dvec3(1.0, 0.0, 1.0)), sun);
	BOOST_CHECK_LE_3(0.0, nextToSunFactor);
	BOOST_CHECK_GE_3(1.0, nextToSunFactor);

	BOOST_CHECK_GE_3(intoSunFactor, nextToSunFactor);
	BOOST_CHECK_LE(nextToSunFactor.r, nextToSunFactor.g);
	BOOST_CHECK_LE(nextToSunFactor.g, nextToSunFactor.b);

	dvec3 closeToHorizonFactor = scatterer.scatteredLight(normalize(dvec3(100.0, 0.0, 1.0)), sun);
	BOOST_CHECK_LE_3(0.0, closeToHorizonFactor);
	BOOST_CHECK_GE_3(1.0, closeToHorizonFactor);
}

BOOST_AUTO_TEST_SUITE_END()
