#include "functions.h"

#include <cmath>

float sqrtt(float x) {
	return sqrtf(x);
}

double sqrtt(double x) {
	return sqrt(x);
}

size_t sqrtt(size_t x) {
	return (size_t)sqrt(x);
}

unsigned nextPowerOfTwo(unsigned n) {
	unsigned p = 1;
	while (p < n)
		p *= 2;
	return p;
}
