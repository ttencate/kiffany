#include "stats.h"

#include <iostream>

Stats stats;

CounterStat::CounterStat()
:
	value(0)
{
}

void CounterStat::increment(long delta) {
	value += delta;
}

long CounterStat::get() const {
	return value;
}

void Stats::print() const {
	std::cout
		<< "Chunks generated: " << chunksGenerated.get() << '\n'
		<< "Quads generated: " << quadsGenerated.get() << '\n'
		<< "Quads per chunk: " << (quadsGenerated.get() / chunksGenerated.get()) << '\n'
		<< '\n'
		<< "Frames rendered: " << framesRendered.get() << '\n'
		<< "Chunks rendered: " << chunksRendered.get() << '\n'
		<< "Quads rendered: " << quadsRendered.get() << '\n'
		<< "Quads per frame: " << (quadsRendered.get() / framesRendered.get()) << '\n';
}
