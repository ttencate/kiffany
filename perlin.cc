#include "perlin.h"

#include <boost/random.hpp>

Noise::Noise(unsigned count, unsigned seed)
:
	noise(count)
{
	boost::mt11213b engine(seed);
	boost::uniform_01<float> uniform;
	for (unsigned i = 0; i < count; ++i) {
		noise[i] = 2.0f * uniform(engine) - 1.0f;
	}
}

Noise2D::Noise2D(unsigned size, unsigned seed)
:
	Noise(size * size, seed),
	size(size)
{
}

float Noise2D::operator()(vec2 pos) const {
	vec2 p = (float)size * fract(pos);
	int xa = (int)p.x;
	int xb = (xa + 1) % size;
	float fx = fract(p.x);
	int ya = (int)p.y;
	int yb = (ya + 1) % size;
	float fy = fract(p.y);
	return mix(
			mix(noise[xa + size * ya], noise[xb + size * ya], fx),
			mix(noise[xa + size * yb], noise[xb + size * yb], fx),
			fy);
}
