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
			BOOST_REQUIRE_LE(0, opticalLength);
			if (a > 0 && angle < M_PI / 2) {
				BOOST_REQUIRE_GE(opticalLength, opticalLengthTable.get(uvec2(layer, a - 1)));
			}
			if (angle < M_PI / 2  && layer > 0) {
				BOOST_REQUIRE_LE(opticalLength, opticalLengthTable.get(uvec2(layer - 1, a)));
			}
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
			dvec3 sunAttenuation = sunAttenuationTable.get(index);
			BOOST_REQUIRE_LE(0, sunAttenuation.r);
			BOOST_REQUIRE_LE(0, sunAttenuation.g);
			BOOST_REQUIRE_LE(0, sunAttenuation.b);
			BOOST_REQUIRE_GE(1, sunAttenuation.r);
			BOOST_REQUIRE_GE(1, sunAttenuation.g);
			BOOST_REQUIRE_GE(1, sunAttenuation.b);
		}
	}
}

BOOST_AUTO_TEST_SUITE_END()
