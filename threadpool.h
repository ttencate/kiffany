#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/noncopyable.hpp>
#include <boost/scoped_array.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/thread.hpp>

// A wrapper around io_service that keeps track of the queue size,
// and allows new posts to block if the queue gets too large.
class WorkQueue
:
	boost::noncopyable
{
	unsigned const maxSize;

	boost::asio::io_service queue;

	unsigned size;
	boost::mutex mutex;
	boost::condition_variable condVar;

	public:

		typedef boost::function<void(void)> Worker;

		WorkQueue(unsigned maxSize);

		boost::asio::io_service::work *createWork();

		// Called on the worker thread
		void run();

		// Called on the main thread
		void reset();
		void stop();

		// Called on either thread
		void post(Worker worker);

	private:

		void runAndDecrementSize(Worker worker);

};

class ThreadPool
:
	boost::noncopyable
{

	unsigned const numThreads;

	WorkQueue inputQueue;
	WorkQueue outputQueue;

	boost::scoped_array<boost::scoped_ptr<boost::asio::io_service::work> > const works;
	boost::scoped_array<boost::scoped_ptr<boost::thread> > const threads;

	public:

		typedef WorkQueue::Worker Worker;
		typedef WorkQueue::Worker Finalizer;

		ThreadPool(unsigned maxInputQueueSize, unsigned maxOutputQueueSize, unsigned requestedNumThreads = 0);
		~ThreadPool();

		void enqueue(Worker worker, Finalizer finalizer);
		void runFinalizers();

	private:

		void work(Worker worker, Finalizer finalizer);
		void finalize(Finalizer finalizer);

};

#endif
