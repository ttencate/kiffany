#include "sky.h"

#include <boost/test/unit_test.hpp>

#include <limits>

#define BOOST_LEVEL_COMP_3(level, comp, a, b) \
	BOOST_##level##_##comp(dvec3(a).x, dvec3(b).x); \
	BOOST_##level##_##comp(dvec3(a).y, dvec3(b).y); \
	BOOST_##level##_##comp(dvec3(a).z, dvec3(b).z)
#define BOOST_CHECK_LE_3(a, b) BOOST_LEVEL_COMP_3(CHECK, LE, a, b)
#define BOOST_CHECK_GE_3(a, b) BOOST_LEVEL_COMP_3(CHECK, GE, a, b)
#define BOOST_REQUIRE_LE_3(a, b) BOOST_LEVEL_COMP_3(REQUIRE, LE, a, b)
#define BOOST_REQUIRE_GE_3(a, b) BOOST_LEVEL_COMP_3(REQUIRE, GE, a, b)

namespace {
	struct Fixture {
		Atmosphere atmosphere;
	};
}

BOOST_FIXTURE_TEST_SUITE(SkyTest, Fixture)

BOOST_AUTO_TEST_CASE(TestComputeLayerHeights) {
	unsigned const numLayers = 10;
	double const rayleighHeight = 50.0;
	double const atmosphereHeight = 1.0e3;
	LayerHeights layerHeights = computeLayerHeights(numLayers, rayleighHeight, atmosphereHeight);
	BOOST_CHECK_EQUAL(numLayers, layerHeights.size());
	double const EPS = 1e-4;
	BOOST_CHECK_EQUAL(0.0, layerHeights[0]);
	for (unsigned i = 0; i < layerHeights.size() - 1; ++i) {
		BOOST_CHECK_LE(0.0, layerHeights[i]);
		BOOST_CHECK_LT(layerHeights[i], layerHeights[i + 1]);
		double cumulativeDensity = rayleighHeight * (1.0 - exp(-layerHeights[i] / rayleighHeight));
		BOOST_CHECK_CLOSE((double)i / layerHeights.size() * rayleighHeight, cumulativeDensity, EPS);
	}
	BOOST_CHECK_EQUAL(atmosphereHeight, layerHeights[numLayers - 1]);
}

BOOST_AUTO_TEST_CASE(TestRayLength) {
	LayerHeights const &layerHeights = atmosphere.getLayerHeights();
	double EPS = 1e-6;
	BOOST_CHECK_CLOSE(350.0, atmosphere.rayLengthToHeight(0.0, 350.0), EPS);
	BOOST_CHECK_LT(350.0, atmosphere.rayLengthToHeight(0.1, 350.0));
	BOOST_CHECK_CLOSE(layerHeights[5], atmosphere.rayLengthToLayer(0.0, 5), EPS);
	BOOST_CHECK_LT(5, atmosphere.rayLengthToLayer(0.1, 5));
	BOOST_CHECK_CLOSE(layerHeights[6] - layerHeights[2], atmosphere.rayLengthBetweenLayers(0.0, 2, 6), EPS);
	BOOST_CHECK_LT(layerHeights[6] - layerHeights[2], atmosphere.rayLengthBetweenLayers(0.1, 2, 6));
}

BOOST_AUTO_TEST_CASE(TestBuildOpticalLengthTable) {
	Dvec3Table2D opticalLengthTable = buildOpticalLengthTable(atmosphere);
	unsigned numLayers = atmosphere.getNumLayers();
	unsigned numAngles = atmosphere.getNumAngles();
	BOOST_REQUIRE_EQUAL(numLayers, opticalLengthTable.getSize().x);
	BOOST_REQUIRE_EQUAL(numAngles, opticalLengthTable.getSize().y);
	for (unsigned a = 0; a < numAngles; ++a) {
		for (unsigned layer = 0; layer < numLayers; ++layer) {
			uvec2 index = uvec2(layer, a);
			dvec3 opticalLength = opticalLengthTable.get(index);
			// Optical length is never negative
			BOOST_REQUIRE_LE_3(0.0, opticalLength);
			// Larger angles mean longer travel distance, hence larger optical length
			if (a > 0) {
				BOOST_REQUIRE_GE_3(opticalLength, opticalLengthTable.get(uvec2(layer, a - 1)));
			}
		}
	}
}

BOOST_AUTO_TEST_CASE(TestBuildOpticalDepthTable) {
	Dvec3Table2D opticalDepthTable = buildOpticalDepthTable(atmosphere);
	unsigned numLayers = atmosphere.getNumLayers();
	unsigned numAngles = atmosphere.getNumAngles();
	BOOST_REQUIRE_EQUAL(numLayers, opticalDepthTable.getSize().x);
	BOOST_REQUIRE_EQUAL(numAngles, opticalDepthTable.getSize().y);
	for (unsigned a = 0; a < numAngles; ++a) {
		for (unsigned layer = 0; layer < numLayers; ++layer) {
			uvec2 index = uvec2(layer, a);
			dvec3 opticalDepth = opticalDepthTable.get(index);
			// Optical length is never negative
			BOOST_REQUIRE_LE_3(0, opticalDepth);
			// Larger angles mean longer travel distance, hence larger optical length
			if (a > 0) {
				BOOST_REQUIRE_GE_3(opticalDepth, opticalDepthTable.get(uvec2(layer, a - 1)));
			}
			// Higher layers mean shorter travel distance, hence smaller optical length
			if (layer > 0) {
				BOOST_REQUIRE_LE_3(opticalDepth, opticalDepthTable.get(uvec2(layer - 1, a)));
			}
		}
	}
}

BOOST_AUTO_TEST_CASE(TestBuildSunAttenuationTable) {
	Dvec3Table2D opticalDepthTable = buildOpticalDepthTable(atmosphere);
	Dvec3Table2D sunAttenuationTable = buildSunAttenuationTable(atmosphere, opticalDepthTable);
	unsigned numLayers = atmosphere.getNumLayers();
	unsigned numAngles = atmosphere.getNumAngles();
	BOOST_REQUIRE_EQUAL(numLayers, sunAttenuationTable.getSize().x);
	BOOST_REQUIRE_EQUAL(numAngles, sunAttenuationTable.getSize().y);
	for (unsigned a = 0; a < numAngles; ++a) {
		for (unsigned layer = 0; layer < numLayers; ++layer) {
			uvec2 index = uvec2(layer, a);
			dvec3 sunAttenuation = sunAttenuationTable.get(index);
			// Attenuation factor should be 0 <= f <= 1
			BOOST_REQUIRE_LE_3(0.0, sunAttenuation);
			BOOST_REQUIRE_GE_3(1.0, sunAttenuation);
			// Larger angles mean longer travel distance, hence smaller attenuation factor
			if (a > 0) {
				BOOST_REQUIRE_LE_3(sunAttenuation, sunAttenuationTable.get(uvec2(layer, a - 1)));
			}
			// Higher layers mean shorter travel distance, hence larger attenuation factor
			if (layer > 0) {
				BOOST_REQUIRE_GE_3(sunAttenuation, sunAttenuationTable.get(uvec2(layer - 1, a)));
			}
		}
	}
}

BOOST_AUTO_TEST_CASE(TestScatterer) {
	Scatterer scatterer(atmosphere);
	dvec3 intoSunFactor = scatterer.scatteredLightFactor(dvec3(0.0, 0.0, 1.0), dvec3(0.0, 0.0, 1.0));
	dvec3 nextToSunFactor = scatterer.scatteredLightFactor(normalize(dvec3(1.0, 0.0, 1.0)), dvec3(0.0, 0.0, 1.0));
	dvec3 closeToHorizonFactor = scatterer.scatteredLightFactor(normalize(dvec3(100.0, 0.0, 1.0)), dvec3(0.0, 0.0, 1.0));
	BOOST_CHECK_LE_3(0.0, intoSunFactor);
	BOOST_CHECK_GE_3(1.0, intoSunFactor);
	BOOST_CHECK_LE_3(0.0, nextToSunFactor);
	BOOST_CHECK_GE_3(1.0, nextToSunFactor);
	BOOST_CHECK_GE_3(intoSunFactor, nextToSunFactor);
	BOOST_CHECK_LE(nextToSunFactor.r, nextToSunFactor.g);
	BOOST_CHECK_LE(nextToSunFactor.g, nextToSunFactor.b);
	BOOST_CHECK_LE_3(0.0, closeToHorizonFactor);
	BOOST_CHECK_GE_3(1.0, closeToHorizonFactor);
}

BOOST_AUTO_TEST_SUITE_END()
