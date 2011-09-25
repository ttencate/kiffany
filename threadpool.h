#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/noncopyable.hpp>
#include <boost/scoped_array.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/thread.hpp>

template<typename ArgType>
class ThreadPool
:
	boost::noncopyable
{
	public:

		typedef ArgType arg_type;
		typedef boost::function<void(ArgType *)> func_type;
	
	private:

		unsigned const size;

		func_type func;

		boost::asio::io_service inputQueue;
		boost::asio::io_service outputQueue;

		boost::scoped_array<boost::scoped_ptr<boost::asio::io_service::work> > works;
		boost::scoped_array<boost::scoped_ptr<boost::thread> > threads;

		std::deque<arg_type> completed;

	public:

		ThreadPool(func_type const &func, unsigned queueSize, unsigned size = 0);
		~ThreadPool();

		void enqueue(arg_type const &arg);
		bool dequeue(arg_type *arg);

	private:

		void doWork(arg_type arg);
		void finishWork(arg_type arg);

};

template<typename ArgType>
ThreadPool<ArgType>::ThreadPool(func_type const &func, unsigned queueSize, unsigned size)
:
	size(size),
	func(func)
{
	if (size == 0) {
		size = boost::thread::hardware_concurrency();
		if (size == 0) {
			size = 1;
		}
	}

	works.reset(new boost::scoped_ptr<boost::asio::io_service::work>[size]);
	threads.reset(new boost::scoped_ptr<boost::thread>[size]);
	for (unsigned i = 0; i < size; ++i) {
		works[i].reset(new boost::asio::io_service::work(inputQueue));
		threads[i].reset(new boost::thread(boost::bind(&boost::asio::io_service::run, &inputQueue)));
	}
}

template<typename ArgType>
ThreadPool<ArgType>::~ThreadPool() {
	inputQueue.stop();
}

template<typename ArgType>
void ThreadPool<ArgType>::enqueue(arg_type const &arg) {
	inputQueue.post(boost::bind(&ThreadPool<ArgType>::doWork, this, arg));
}

template<typename ArgType>
bool ThreadPool<ArgType>::dequeue(arg_type *arg) {
	outputQueue.run();
	outputQueue.reset();
	if (completed.size()) {
		*arg = completed.front();
		completed.pop_front();
		return true;
	} else {
		return false;
	}
}

template<typename ArgType>
void ThreadPool<ArgType>::doWork(arg_type arg) {
	func(&arg);
	outputQueue.post(boost::bind(&ThreadPool<ArgType>::finishWork, this, arg));
}

template<typename ArgType>
void ThreadPool<ArgType>::finishWork(arg_type arg) {
	completed.push_back(arg);
}

#endif
