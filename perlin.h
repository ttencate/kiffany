#ifndef PERLIN_H
#define PERLIN_H

#include "maths.h"
#include "table.h"

#include <boost/random.hpp>

#include <vector>

template<typename T>
void fillWithNoise(Table<T> &table, unsigned seed) {
	boost::mt11213b engine(seed);
	boost::uniform_01<T> uniform;
	T const *const end = table.raw() + table.getSize();
	for (T *p = table.raw(); p < end; ++p) {
		*p = 2 * uniform(engine) - 1;
	}
}

template<typename T>
Table2D<T> buildNoise2D(unsigned nx, unsigned ny, unsigned seed) {
	Table2D<T> noise(nx, ny);
	fillWithNoise(noise, seed);
	return noise;
}

template<typename T>
Table3D<T> buildNoise3D(unsigned nx, unsigned ny, unsigned nz, unsigned seed) {
	Table3D<T> noise(nx, ny, nz);
	fillWithNoise(noise, seed);
	return noise;
}

struct Octave {
	float frequency;
	float amplitude;
	Octave(float period, float amplitude)
	:
		frequency(1.0f / period),
		amplitude(amplitude)
	{
	}
};

typedef std::vector<Octave> Octaves;

template<typename TableType>
class Perlin {

	TableType noise;
	Octaves octaves;

	float amplitude;

	public:

		typedef typename TableType::Coords Coords;

		Perlin(TableType const &table, Octaves const &octaves)
		:
			noise(table),
			octaves(octaves)
		{
			amplitude = 0;
			for (unsigned i = 0; i < octaves.size(); ++i) {
				amplitude += this->octaves[i].amplitude;
			}
		}

		float operator()(Coords pos) const {
			float out = 0;
			for (unsigned i = 0; i < octaves.size(); ++i) {
				Octave const &octave = octaves[i];
				out += octave.amplitude * noise(octave.frequency * pos);
			}
			return out;
		}

		float getAmplitude() const {
			return amplitude;
		}

};

typedef Perlin<Table2D<float> > Perlin2D;
typedef Perlin<Table3D<float> > Perlin3D;

#endif
