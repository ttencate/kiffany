#ifndef PERLIN_H
#define PERLIN_H

#include "maths.h"
#include "table.h"

#include <boost/random.hpp>

#include <vector>

template<typename T>
class NoiseGenerator {
	mutable boost::mt11213b engine;
	mutable boost::uniform_01<T> uniform;
	public:
		NoiseGenerator(unsigned seed)
		:
			engine(seed)
		{
		}
		T operator()() const {
			return 2 * uniform(engine) - 1;
		}
};

template<typename TableType>
TableType buildNoiseTable(typename TableType::size_type size, unsigned seed) {
	TableType noise(size);
	NoiseGenerator<typename TableType::value_type> gen(seed);
	std::generate(noise.begin(), noise.end(), gen);
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

		typedef typename TableType::coords_type coords_type;

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

		float operator()(coords_type pos) const {
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

typedef Perlin<FloatTable2D> Perlin2D;
typedef Perlin<FloatTable3D> Perlin3D;

#endif
