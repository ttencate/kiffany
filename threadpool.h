#ifndef THREADPOOL_H
#define THREADPOOL_H

#include "queues.h"
#include "threading.h"

#include <boost/bind.hpp>
#include <boost/noncopyable.hpp>
#include <boost/scoped_array.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/thread.hpp>

#include <queue>

// A thread-safe wrapper around priority_queue,
// and allows new posts to block if the queue gets too large.
class WorkQueue
:
	boost::noncopyable
{
	public:

		typedef boost::function<void(void)> Worker;
		typedef float Priority;

	private:

		typedef ExplicitPriorityQueue<Worker> Queue;

		unsigned const maxSize;

		boost::mutex mutable queueMutex;
		Queue queue;
		boost::condition_variable conditionVariable;
		Semaphore semaphore;

	public:

		WorkQueue(unsigned maxSize);

		void post(Worker worker, Priority priority = Priority());
		bool tryPost(Worker worker, Priority priority = Priority());

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

		typedef WorkQueue::Priority Priority;
		typedef boost::function<void(void)> Finalizer;
		typedef boost::function<Finalizer(void)> Worker;

		ThreadPool(unsigned maxInputQueueSize, unsigned maxOutputQueueSize, unsigned numThreads = defaultNumThreads());
		~ThreadPool();

		void enqueue(Worker worker, Priority priority = Priority());
		bool tryEnqueue(Worker worker, Priority priority = Priority());
		void runFinalizers();

		static unsigned defaultNumThreads();

	private:

		void loop();
		void work(Worker worker);
		void finalize(Finalizer finalizer);

};

#endif
