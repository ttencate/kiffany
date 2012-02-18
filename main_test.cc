#include "flags.h"

#define BOOST_TEST_MAIN

#include <boost/test/unit_test.hpp>

namespace {
	struct FlagsFixture {
		FlagsFixture() {
			setDefaultFlags();
		}
	};
}
BOOST_GLOBAL_FIXTURE(FlagsFixture)
