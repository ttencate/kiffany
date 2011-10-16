#ifndef THREADPOOL_H
#define THREADPOOL_H

#include "queues.h"
#include "threading.h"

#include <boost/bind.hpp>
#include <boost/noncopyable.hpp>
#include <boost/scoped_array.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/thread.hpp>

#include <iostream> // TODO remove
#include <queue>

/* A thread-safe wrapper around a priority queue,
 * which also allows new posts to block if the queue gets too large.
 */
template<typename WorkerType = boost::function<void(void)>, typename QueueType = DynamicPriorityQueue<WorkerType> >
class WorkQueue
:
	boost::noncopyable
{
	public:

		typedef WorkerType Worker;
		typedef float Priority;
		typedef boost::function<Priority(Worker const &)> PriorityFunction;

	private:

		typedef QueueType Queue;

		unsigned const maxSize;

		boost::mutex mutable queueMutex;
		Queue queue;
		boost::condition_variable conditionVariable;
		Semaphore semaphore;

	public:

		WorkQueue(unsigned maxSize)
		:
			maxSize(maxSize),
			semaphore(maxSize == 0 ? std::numeric_limits<unsigned>::max() : maxSize)
		{
		}

		void post(Worker worker) {
			semaphore.wait();
			doPost(worker);
		}

		bool tryPost(Worker worker) {
			if (semaphore.tryWait()) {
				doPost(worker);
				return true;
			} else {
				return false;
			}
		}

		void runOne() {
			boost::unique_lock<boost::mutex> lock(queueMutex);
			while (queue.empty()) {
				conditionVariable.wait(lock);
			}
			Worker worker(queue.front());
			queue.pop_front();
			lock.unlock();

			semaphore.signal();
			worker();
		}

		void runAll() {
			Queue all;
			{
				boost::unique_lock<boost::mutex> lock(queueMutex);
				std::swap(queue, all);
			}
			for (unsigned i = 0; i < all.size(); ++i) {
				semaphore.signal();
			}
			while (!all.empty()) {
				Worker worker = all.front();
				all.pop_front();
				worker();
			}
		}

		void setPriorityFunction(PriorityFunction const &priorityFunction) {
			boost::unique_lock<boost::mutex> lock(queueMutex);
			queue.setPriorityFunction(priorityFunction);
		}

	private:

		void doPost(Worker worker) {
			{
				boost::unique_lock<boost::mutex> lock(queueMutex);
				queue.insert(worker);
			}
			conditionVariable.notify_one();
		}

};

template<typename WorkerType = boost::function<void(void)> >
class ThreadPool
:
	boost::noncopyable
{

	public:

		typedef WorkerType Worker;
		typedef float Priority;

	private:

		typedef WorkQueue<Worker> Queue;
		typedef ThreadPool<Worker> Type;

		unsigned const numThreads;

		Queue queue;

		boost::scoped_array<boost::scoped_ptr<boost::thread> > const threads;

	public:

		typedef typename Queue::PriorityFunction PriorityFunction;

		ThreadPool(unsigned maxQueueSize, unsigned numThreads = defaultNumThreads())
		:
			numThreads(numThreads == 0 ? defaultNumThreads() : numThreads),
			queue(maxQueueSize),
			threads(new boost::scoped_ptr<boost::thread>[numThreads])
		{
			for (unsigned i = 0; i < numThreads; ++i) {
				threads[i].reset(new boost::thread(boost::bind(&Type::loop, this)));
			}
		}

		~ThreadPool() {
			for (unsigned i = 0; i < numThreads; ++i) {
				threads[i]->interrupt();
			}
			// Do not delete the queue until all threads have finished.
			for (unsigned i = 0; i < numThreads; ++i) {
				threads[i]->join();
			}
		}

		void enqueue(Worker worker) {
			queue.post(worker);
		}

		bool tryEnqueue(Worker worker) {
			return queue.tryPost(worker);
		}

		static unsigned defaultNumThreads() {
			unsigned numThreads = boost::thread::hardware_concurrency();
			if (numThreads == 0) {
				numThreads = 1;
			}
			return numThreads;
		}

		void setPriorityFunction(PriorityFunction const &priorityFunction) {
			queue.setPriorityFunction(priorityFunction);
		}

	private:

		void loop() {
			while (true) {
				queue.runOne();
			}
		}

};

#endif
