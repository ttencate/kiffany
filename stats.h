#ifndef STATS_H
#define STATS_H

#include <boost/noncopyable.hpp>
#include <boost/thread.hpp>

#include <ctime>

template<typename T>
class Stat {

	boost::shared_mutex mutable mutex;
	T value;

	public:

		Stat()
		:
			value(0)
		{
		}

		void increment(T delta = 1) {
			boost::unique_lock<boost::shared_mutex> lock(mutex);
			value += delta;
		}

		T get() const {
			boost::shared_lock<boost::shared_mutex> lock(mutex);
			return value;
		}

};

typedef Stat<unsigned long> CounterStat;

class TimerStat
:
	Stat<double>
{

	public:

		class Timed {

			friend class TimerStat;

			mutable TimerStat *parent;
			timespec start;

			public:

				Timed(Timed const &other)
				:
					parent(other.parent),
					start(other.start)
				{
					other.parent = 0;
				}

				~Timed() {
					if (parent) {
						timespec end;
						clock_gettime(CLOCK_MONOTONIC, &end);
						double delta = end.tv_sec - start.tv_sec + 1e-9 * (end.tv_nsec - start.tv_nsec);
						parent->increment(delta);
					}
				}

			private:

				Timed(TimerStat *parent)
				:
					parent(parent)
				{
					clock_gettime(CLOCK_MONOTONIC, &start);
				}

				Timed &operator=(Timed const &other); // NI: not needed
		};

		Timed timed() {
			return Timed(this);
		}

		using Stat<double>::get;

};

struct Stats {
	TimerStat runningTime;

	CounterStat octreeNodes;
	CounterStat quadsGenerated;

	CounterStat chunksCreated;
	CounterStat chunksEvicted;

	CounterStat chunksGenerated;
	TimerStat chunkGenerationTime;
	CounterStat octreesBuilt;
	TimerStat octreeBuildTime;
	CounterStat octreesUnpacked;
	TimerStat octreeUnpackTime;
	CounterStat chunksTesselated;
	TimerStat chunkTesselationTime;

	CounterStat irrelevantJobsSkipped;
	CounterStat irrelevantJobsRun;

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
