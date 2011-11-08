#include <boost/test/unit_test.hpp>

#include "sky.h"

#include <limits>

BOOST_AUTO_TEST_SUITE(SkyTest)

BOOST_AUTO_TEST_CASE(TestBuildOpticalLengthTable) {
	Atmosphere atmosphere;
	DoubleTable2D opticalLengthTable = buildOpticalLengthTable(atmosphere);
	unsigned numLayers = atmosphere.getNumLayers();
	unsigned numAngles = atmosphere.getNumAngles();
	BOOST_REQUIRE_EQUAL(numLayers, opticalLengthTable.getSize().x);
	BOOST_REQUIRE_EQUAL(numAngles, opticalLengthTable.getSize().y);
	for (unsigned a = 0; a < numAngles; ++a) {
		for (unsigned layer = 0; layer < numLayers; ++layer) {
			uvec2 index = uvec2(layer, a);
			double angle = opticalLengthTable.coordsFromIndex(index).y;
			double opticalLength = opticalLengthTable.get(index);
			// Optical length is never negative
			BOOST_REQUIRE_LE(0, opticalLength);
			// Larger angles mean longer travel distance, hence larger optical length
			if (a > 0) {
				BOOST_REQUIRE_GE(opticalLength, opticalLengthTable.get(uvec2(layer, a - 1)));
			}
			// Higher layers mean shorter travel distance, hence smaller optical length
			if (layer > 0) {
				BOOST_REQUIRE_LE(opticalLength, opticalLengthTable.get(uvec2(layer - 1, a)));
			}
			// Angles beyond 90 degrees go through the earth, hence infinite optical length
			if (angle >= M_PI / 2) {
				BOOST_REQUIRE_EQUAL(std::numeric_limits<double>::infinity(), opticalLength);
			}
		}
	}
}

BOOST_AUTO_TEST_CASE(TestBuildSunAttenuationTable) {
	Atmosphere atmosphere;
	DoubleTable2D opticalLengthTable = buildOpticalLengthTable(atmosphere);
	Dvec3Table2D sunAttenuationTable = buildSunAttenuationTable(atmosphere, opticalLengthTable);
	unsigned numLayers = atmosphere.getNumLayers();
	unsigned numAngles = atmosphere.getNumAngles();
	BOOST_REQUIRE_EQUAL(numLayers, sunAttenuationTable.getSize().x);
	BOOST_REQUIRE_EQUAL(numAngles, sunAttenuationTable.getSize().y);
	for (unsigned a = 0; a < numAngles; ++a) {
		for (unsigned layer = 0; layer < numLayers; ++layer) {
			uvec2 index = uvec2(layer, a);
			double angle = sunAttenuationTable.coordsFromIndex(index).y;
			dvec3 sunAttenuation = sunAttenuationTable.get(index);
			// Attenuation factor should be 0 <= f <= 1
			BOOST_REQUIRE_LE(0, sunAttenuation.r);
			BOOST_REQUIRE_LE(0, sunAttenuation.g);
			BOOST_REQUIRE_LE(0, sunAttenuation.b);
			BOOST_REQUIRE_GE(1, sunAttenuation.r);
			BOOST_REQUIRE_GE(1, sunAttenuation.g);
			BOOST_REQUIRE_GE(1, sunAttenuation.b);
			// Larger angles mean longer travel distance, hence smaller attenuation factor
			if (a > 0) {
				BOOST_REQUIRE_LE(sunAttenuation.r, sunAttenuationTable.get(uvec2(layer, a - 1)).r);
				BOOST_REQUIRE_LE(sunAttenuation.g, sunAttenuationTable.get(uvec2(layer, a - 1)).g);
				BOOST_REQUIRE_LE(sunAttenuation.b, sunAttenuationTable.get(uvec2(layer, a - 1)).b);
			}
			// Higher layers mean shorter travel distance, hence larger attenuation factor
			if (layer > 0) {
				BOOST_REQUIRE_GE(sunAttenuation.r, sunAttenuationTable.get(uvec2(layer - 1, a)).x);
				BOOST_REQUIRE_GE(sunAttenuation.g, sunAttenuationTable.get(uvec2(layer - 1, a)).y);
				BOOST_REQUIRE_GE(sunAttenuation.b, sunAttenuationTable.get(uvec2(layer - 1, a)).b);
			}
			// Angles beyond 90 degrees go through the earth, hence zero attenuation factor
			if (angle >= M_PI / 2) {
				BOOST_REQUIRE_EQUAL(0, sunAttenuation.r);
				BOOST_REQUIRE_EQUAL(0, sunAttenuation.g);
				BOOST_REQUIRE_EQUAL(0, sunAttenuation.b);
			}
		}
	}
}

BOOST_AUTO_TEST_SUITE_END()
