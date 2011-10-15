#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <boost/bind.hpp>
#include <boost/noncopyable.hpp>
#include <boost/scoped_array.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/thread.hpp>

#include <queue>

class Semaphore
:
	boost::noncopyable
{
	unsigned count;
	boost::mutex mutex;
	boost::condition_variable condition_variable;

	public:

		Semaphore(unsigned initialCount);

		void wait();
		bool tryWait();
		void signal();

};

typedef float Priority;

extern Priority const DEFAULT_PRIORITY;

// A wrapper around io_service that keeps track of the queue size,
// and allows new posts to block if the queue gets too large.
class WorkQueue
:
	boost::noncopyable
{
	public:

		typedef boost::function<void(void)> Worker;

	private:

		typedef std::pair<Worker, Priority> PriorityWorker;
		struct PriorityWorkerCompare {
			bool operator()(PriorityWorker const &a, PriorityWorker const &b) {
				return a.second < b.second;
			}
		};
		typedef std::priority_queue<PriorityWorker, std::vector<PriorityWorker>, PriorityWorkerCompare> Queue;

		unsigned const maxSize;

		boost::mutex mutable queueMutex;
		Queue queue;
		boost::condition_variable conditionVariable;
		Semaphore semaphore;

	public:

		WorkQueue(unsigned maxSize);

		void post(Worker worker, Priority priority = DEFAULT_PRIORITY);
		bool tryPost(Worker worker, Priority priority = DEFAULT_PRIORITY);

		bool empty() const;

		void runOne();
		void runAll();

	private:

		void doPost(Worker worker, Priority priority);

};

class ThreadPool
:
	boost::noncopyable
{

	unsigned const numThreads;

	WorkQueue inputQueue;
	WorkQueue outputQueue;

	boost::scoped_array<boost::scoped_ptr<boost::thread> > const threads;

	public:

		typedef boost::function<void(void)> Finalizer;
		typedef boost::function<Finalizer(void)> Worker;

		ThreadPool(unsigned maxInputQueueSize, unsigned maxOutputQueueSize, unsigned numThreads = defaultNumThreads());
		~ThreadPool();

		void enqueue(Worker worker, Priority priority = DEFAULT_PRIORITY);
		bool tryEnqueue(Worker worker, Priority priority = DEFAULT_PRIORITY);
		void runFinalizers();

		static unsigned defaultNumThreads();

	private:

		void loop();
		void work(Worker worker);
		void finalize(Finalizer finalizer);

};

#endif
