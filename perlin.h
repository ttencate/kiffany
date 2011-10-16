#ifndef PERLIN_H
#define PERLIN_H

#include "maths.h"

#include <vector>

class Noise {

	protected:

		std::vector<float> noise;

	public:

		Noise(unsigned count, unsigned seed);

};

class Noise2D
:
	Noise
{

	unsigned const size;

	public:

		typedef vec2 Coords;

		Noise2D(unsigned size, unsigned seed);

		unsigned getSize() const { return size; }

		float operator()(vec2 pos) const;

};

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

template<typename NoiseType>
class Perlin {

	NoiseType noise;
	Octaves octaves;

	public:

		typedef typename NoiseType::Coords Coords;

		Perlin(NoiseType const &noise, Octaves const &octaves)
		:
			noise(noise),
			octaves(octaves)
		{
			for (unsigned i = 0; i < octaves.size(); ++i) {
				this->octaves[i].frequency /= noise.getSize();
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

};

#endif
