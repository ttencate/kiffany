#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include <cstddef>

template<typename T>
void clamp(T const &min, T &t, T const &max) {
	if (t < min)
		t = min;
	else if (t > max)
		t = max;
}

template<typename T>
T sqr(T const &x) {
	return x * x;
}

float sqrtt(float x);

double sqrtt(double x);

size_t sqrtt(size_t x);

template<typename T>
T sign(T const &x) {
	if (x > 0) return 1;
	if (x < 0) return -1;
	return 0;
}

unsigned nextPowerOfTwo(unsigned n);

#ifndef PI
#define PI 3.14159265358979323846
#endif

#endif
