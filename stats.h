#ifndef STATS_H
#define STATS_H

class CounterStat {

	long value;

	public:

		CounterStat();
		void increment(long delta = 1);
		long get() const;

};

struct Stats {
	CounterStat chunksGenerated;
	CounterStat quadsGenerated;
	CounterStat framesRendered;
	CounterStat chunksRendered;
	CounterStat quadsRendered;

	void print() const;
};

extern Stats stats;

#endif
