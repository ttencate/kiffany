#include <boost/test/unit_test.hpp>
#include <boost/test/floating_point_comparison.hpp>

#include "table.h"

BOOST_AUTO_TEST_SUITE(TableTest)

BOOST_AUTO_TEST_CASE(TestTable) {
	Table<int> table(2);
	table[0] = 5;
	table[1] = 10;
	Table<int> const &ref = table;
	BOOST_CHECK_EQUAL(2, table.getSize());
	BOOST_CHECK_EQUAL(5, ref[0]);
	BOOST_CHECK_EQUAL(10, ref[1]);
}

BOOST_AUTO_TEST_CASE(TestCopyTable) {
	Table<int> table(2);
	table[0] = 5;
	table[1] = 10;
	Table<int> copy(table);
	BOOST_CHECK_EQUAL(table.getSize(), copy.getSize());
	BOOST_CHECK_EQUAL(table[0], copy[0]);
	BOOST_CHECK_EQUAL(table[1], copy[1]);
}

BOOST_AUTO_TEST_CASE(TestInterpolation2D) {
	Table2D<float> table(2, 2);
	table[0] = 1;
	table[1] = 2;
	table[2] = 3;
	table[3] = 4;
	float const EPS = 1e-6;
	BOOST_CHECK_CLOSE(1.0f, table(vec2(0.5f, 0.5f)), EPS);
	BOOST_CHECK_CLOSE(2.0f, table(vec2(1.5f, 0.5f)), EPS);
	BOOST_CHECK_CLOSE(3.0f, table(vec2(0.5f, 1.5f)), EPS);
	BOOST_CHECK_CLOSE(4.0f, table(vec2(1.5f, 1.5f)), EPS);
	BOOST_CHECK_CLOSE(1.75f, table(vec2(1.25f, 0.5f)), EPS);
	BOOST_CHECK_CLOSE(1.5f, table(vec2(0.5f, 0.75f)), EPS);
	BOOST_CHECK_CLOSE(1.25f, table(vec2(0.25f, 0.5f)), EPS);
	BOOST_CHECK_CLOSE(2.0f, table(vec2(0.5f, 0.0f)), EPS);
	BOOST_CHECK_CLOSE(2.5f, table(vec2(0.0f, 0.0f)), EPS);
	BOOST_CHECK_CLOSE(2.5f, table(vec2(1.0f, 1.0f)), EPS);
}

BOOST_AUTO_TEST_CASE(TestInterpolation3D) {
	Table3D<float> table(2, 2, 2);
	table[0] = 1;
	table[1] = 2;
	table[2] = 3;
	table[3] = 4;
	table[4] = 5;
	table[5] = 6;
	table[6] = 7;
	table[7] = 8;
	float const EPS = 1e-6;
	BOOST_CHECK_CLOSE(1.0f, table(vec3(0.5f, 0.5f, 0.5f)), EPS);
	BOOST_CHECK_CLOSE(2.0f, table(vec3(1.5f, 0.5f, 0.5f)), EPS);
	BOOST_CHECK_CLOSE(3.0f, table(vec3(0.5f, 1.5f, 0.5f)), EPS);
	BOOST_CHECK_CLOSE(8.0f, table(vec3(1.5f, 1.5f, 1.5f)), EPS);
	BOOST_CHECK_CLOSE(1.5f, table(vec3(1.0f, 0.5f, 0.5f)), EPS);
	BOOST_CHECK_CLOSE(2.0f, table(vec3(0.5f, 1.0f, 0.5f)), EPS);
	BOOST_CHECK_CLOSE(3.0f, table(vec3(0.5f, 0.5f, 1.0f)), EPS);
	BOOST_CHECK_CLOSE(1.5f, table(vec3(0.0f, 0.5f, 0.5f)), EPS);
	BOOST_CHECK_CLOSE(2.0f, table(vec3(0.5f, 0.0f, 0.5f)), EPS);
	BOOST_CHECK_CLOSE(3.0f, table(vec3(0.5f, 0.5f, 0.0f)), EPS);
	BOOST_CHECK_CLOSE(2.5f, table(vec3(0.0f, 0.0f, 0.5f)), EPS);
	BOOST_CHECK_CLOSE(2.5f, table(vec3(1.0f, 1.0f, 0.5f)), EPS);
	BOOST_CHECK_CLOSE(4.5f, table(vec3(1.0f, 1.0f, 1.0f)), EPS);
	BOOST_CHECK_CLOSE(4.5f, table(vec3(0.0f, 0.0f, 0.0f)), EPS);
}

BOOST_AUTO_TEST_SUITE_END()
