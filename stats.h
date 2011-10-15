#ifndef STATS_H
#define STATS_H

#include <boost/noncopyable.hpp>
#include <boost/thread.hpp>

#include <ctime>

template<typename T>
class UnsafeStat {

	T value;

	public:

		UnsafeStat()
		:
			value(0)
		{
		}

		void increment(T delta = 1) {
			value += delta;
		}

		T get() const {
			return value;
		}

};

template<typename T>
class SafeStat {

	boost::shared_mutex mutable mutex;
	T value;

	public:

		SafeStat()
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

typedef UnsafeStat<long> UnsafeCounter;
typedef SafeStat<long> SafeCounter;

template<typename StatType>
class TimerStat
:
	StatType
{

	public:

		class Timed {

			friend class TimerStat<StatType>;

			mutable TimerStat<StatType> *parent;
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

				Timed(TimerStat<StatType> *parent)
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

		using StatType::get;

};

typedef TimerStat<UnsafeStat<double> > UnsafeTimer;
typedef TimerStat<SafeStat<double> > SafeTimer;

struct Stats {
	UnsafeTimer runningTime;

	UnsafeCounter buffersCreated;
	UnsafeCounter buffersDeleted;

	SafeCounter chunksCreated;
	SafeCounter chunksGenerated;
	SafeCounter chunksEvicted;
	SafeCounter quadsGenerated;
	SafeTimer chunkGenerationTime;

	SafeCounter chunksTesselated;
	SafeTimer chunkTesselationTime;

	UnsafeCounter framesRendered;
	UnsafeCounter chunksConsidered;
	UnsafeCounter chunksSkipped;
	UnsafeCounter chunksCulled;
	UnsafeCounter chunksEmpty;
	UnsafeCounter chunksRendered;
	UnsafeCounter quadsRendered;

	void print() const;
};

extern Stats stats;

#endif
