#include "flags.h"

namespace {
	bool dummy = setDefaultFlags();
}

#define BOOST_TEST_MAIN

#include <boost/test/unit_test.hpp>
