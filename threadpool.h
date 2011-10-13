#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/noncopyable.hpp>
#include <boost/scoped_array.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/thread.hpp>

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

// A wrapper around io_service that keeps track of the queue size,
// and allows new posts to block if the queue gets too large.
class WorkQueue
:
	boost::noncopyable
{
	unsigned const maxSize;

	boost::asio::io_service queue;
	Semaphore semaphore;

	public:

		typedef boost::function<void(void)> Worker;

		WorkQueue(unsigned maxSize);

		boost::asio::io_service::work *createWork();

		void run();

		void reset();
		void stop();

		void post(Worker worker);
		bool tryPost(Worker worker);

	private:

		void runAndSignal(Worker worker);

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

		typedef boost::function<void(void)> Finalizer;
		typedef boost::function<Finalizer(void)> Worker;

		ThreadPool(unsigned maxInputQueueSize, unsigned maxOutputQueueSize, unsigned requestedNumThreads = 0);
		~ThreadPool();

		void enqueue(Worker worker);
		bool tryEnqueue(Worker worker);
		void runFinalizers();

	private:

		void work(Worker worker);
		void finalize(Finalizer finalizer);

};

#endif
