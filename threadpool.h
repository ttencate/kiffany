#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/noncopyable.hpp>
#include <boost/scoped_array.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/thread.hpp>

class ThreadPool
:
	boost::noncopyable
{

	unsigned const size;

	boost::asio::io_service inputQueue;
	boost::asio::io_service outputQueue;

	boost::scoped_array<boost::scoped_ptr<boost::asio::io_service::work> > works;
	boost::scoped_array<boost::scoped_ptr<boost::thread> > threads;

	public:

		typedef boost::function<void(void)> Worker;
		typedef boost::function<void(void)> Finalizer;

		ThreadPool(unsigned requestedSize = 0);
		~ThreadPool();

		void enqueue(Worker worker, Finalizer finalizer);
		void runFinalizers();

	private:

		void work(Worker worker, Finalizer finalizer);
		void finalize(Finalizer finalizer);

};

#endif
