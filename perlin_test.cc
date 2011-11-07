#include <boost/test/unit_test.hpp>
#include <boost/test/floating_point_comparison.hpp>

#include "perlin.h"

BOOST_AUTO_TEST_SUITE(PerlinTest)

BOOST_AUTO_TEST_CASE(TestFillWithNoise) {
	Table<float> table(1024);
	fillWithNoise(table, 4);
	bool seenLarge = false;
	bool seenSmall = false;
	for (unsigned i = 0; i < table.getSize(); ++i) {
		BOOST_REQUIRE(table[i] >= -1.0f);
		BOOST_REQUIRE(table[i] <= 1.0f);
		seenLarge |= table[i] > 0.5f;
		seenSmall |= table[i] < -0.5f;
	}
	BOOST_REQUIRE(seenLarge);
	BOOST_REQUIRE(seenSmall);
}

Table2D<float> smallTable() {
	Table2D<float> table(2, 2);
	table[0] = 0;
	table[1] = 1;
	table[2] = 1;
	table[3] = 0;
	return table;
}

BOOST_AUTO_TEST_CASE(TestPerlin2DSingleOctave) {
	Table2D<float> table = smallTable();
	Octaves octaves;
	octaves.push_back(Octave(1.0f, 1.0f));
	Perlin2D perlin(table, octaves);

	float const EPS = 1e-6;
	BOOST_CHECK_CLOSE(1.0f, perlin.getAmplitude(), EPS);
	BOOST_CHECK_CLOSE(0.5f, perlin(vec2(0.5f, 0.25f)), EPS);
	BOOST_CHECK_CLOSE(0.5f, perlin(vec2(0.0f, 0.25f)), EPS);
}

BOOST_AUTO_TEST_CASE(TestPerlin2DMultipleOctaves) {
	Table2D<float> table = smallTable();
	Octaves octaves;
	octaves.push_back(Octave(1.0f, 1.0f));
	octaves.push_back(Octave(0.5f, 0.5f));
	Perlin2D perlin(table, octaves);

	float const EPS = 1e-6;
	BOOST_CHECK_CLOSE(1.5f, perlin.getAmplitude(), EPS);
	BOOST_CHECK_CLOSE(0.75f, perlin(vec2(0.5f, 0.25f)), EPS);
	BOOST_CHECK_CLOSE(0.75f, perlin(vec2(0.0f, 0.25f)), EPS);
}

BOOST_AUTO_TEST_SUITE_END()
