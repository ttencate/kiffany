#ifndef STATS_H
#define STATS_H

#include <ctime>

class CounterStat {

	long value;

	public:

		CounterStat();
		void increment(long delta = 1);
		long get() const;

};

class TimerStat;

class Timed {

	friend class TimerStat;

	TimerStat *parent;
	timespec start;

	public:

		Timed(Timed const &other);
		~Timed();

	private:

		Timed(TimerStat *parent);

		Timed &operator=(Timed &); // NI
};

class TimerStat {

	friend class Timed;

	double value;

	public:

		TimerStat();
		Timed timed();
		double get() const;

};

struct Stats {
	TimerStat runningTime;

	CounterStat buffersCreated;
	CounterStat buffersDeleted;

	CounterStat chunksGenerated;
	CounterStat chunksEvicted;
	CounterStat quadsGenerated;
	TimerStat chunkGenerationTime;

	CounterStat chunksTesselated;
	TimerStat chunkTesselationTime;

	CounterStat framesRendered;
	CounterStat chunksConsidered;
	CounterStat chunksSkipped;
	CounterStat chunksCulled;
	CounterStat chunksEmpty;
	CounterStat chunksRendered;
	CounterStat quadsRendered;

	void print() const;
};

extern Stats stats;

#endif
