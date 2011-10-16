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
	int ya = (int)p.y;
	int yb = (ya + 1) % size;
	vec2 f = fract(p);
	return mix(
			mix(noise[xa + size * ya], noise[xb + size * ya], f.x),
			mix(noise[xa + size * yb], noise[xb + size * yb], f.x),
			f.y);
}

Noise3D::Noise3D(unsigned size, unsigned seed)
:
	Noise(size * size * size, seed),
	size(size)
{
}

float Noise3D::operator()(vec3 pos) const {
	int const FY = size;
	int const FZ = size * size;
	vec3 p = (float)size * fract(pos);
	int xa = (int)p.x;
	int xb = (xa + 1) % size;
	int ya = (int)p.y;
	int yb = (ya + 1) % size;
	int za = (int)p.z;
	int zb = (za + 1) % size;
	vec3 f = fract(p);
	return mix(
			mix(
				mix(noise[xa + FY * ya + FZ * za], noise[xb + FY * ya + FZ * za], f.x),
				mix(noise[xa + FY * yb + FZ * za], noise[xb + FY * yb + FZ * za], f.x),
				f.y),
			mix(
				mix(noise[xa + FY * ya + FZ * zb], noise[xb + FY * ya + FZ * zb], f.x),
				mix(noise[xa + FY * yb + FZ * zb], noise[xb + FY * yb + FZ * zb], f.x),
				f.y),
			f.z);
}
