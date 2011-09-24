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

TimerStat::TimerStat()
:
	value(0.0)
{
}

Timed::Timed(TimerStat *parent)
:
	parent(parent)
{
	clock_gettime(CLOCK_MONOTONIC, &start);
}

Timed::~Timed() {
	if (parent) {
		timespec end;
		clock_gettime(CLOCK_MONOTONIC, &end);
		double delta = end.tv_sec - start.tv_sec + 1e-9 * (end.tv_nsec - start.tv_nsec);
		parent->value += delta;
	}
}

Timed TimerStat::timed() {
	return Timed(this);
}

double TimerStat::get() const {
	return value;
}

void Stats::print() const {
	std::cout
		<< "Chunks generated: " << chunksGenerated.get() << '\n'
		<< "Quads generated: " << quadsGenerated.get() << '\n'
		<< "Quads per chunk: " << (quadsGenerated.get() / chunksGenerated.get()) << '\n'
		<< '\n'
		<< "Running time: " << runningTime.get() << '\n'
		<< "Frames rendered: " << framesRendered.get() << '\n'
		<< "Frames per second: " << (framesRendered.get() / runningTime.get()) << '\n'
		<< '\n'
		<< "Chunks rendered: " << chunksRendered.get() << '\n'
		<< "Quads rendered: " << quadsRendered.get() << '\n'
		<< "Quads per frame: " << (quadsRendered.get() / framesRendered.get()) << '\n';
}
