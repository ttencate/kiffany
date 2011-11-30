#ifndef THREADPOOL_H
#define THREADPOOL_H

#include "threading.h"

#include <boost/bind.hpp>
#include <boost/noncopyable.hpp>
#include <boost/scoped_array.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/thread.hpp>

#include <deque>

/* A thread-safe fifo queue, which also allows new posts to block if the queue gets too large.
 */
class WorkQueue
:
	boost::noncopyable
{
	public:

		typedef boost::function<void(void)> Worker;

	private:

		typedef std::deque<Worker> Deque;

		unsigned const maxSize;

		boost::mutex mutable queueMutex;
		Deque deque;
		boost::condition_variable conditionVariable;
		Semaphore semaphore;

	public:

		WorkQueue(unsigned maxSize);

		void post(Worker worker);
		bool tryPost(Worker worker);

		void runOne();
		void runAll();

		unsigned getSize() const;
		unsigned getMaxSize() const;

	private:

		void doPost(Worker worker);

};

class ThreadPool
:
	boost::noncopyable
{
	unsigned const numThreads;

	WorkQueue queue;

	boost::scoped_array<boost::scoped_ptr<boost::thread> > const threads;

	public:

		typedef WorkQueue::Worker Worker;

		ThreadPool(unsigned maxQueueSize, unsigned numThreads = defaultNumThreads());
		~ThreadPool();

		void enqueue(Worker worker);
		bool tryEnqueue(Worker worker);

		unsigned getQueueSize() const;
		unsigned getMaxQueueSize() const;

		static unsigned defaultNumThreads();

	private:

		void loop();

};

#endif
