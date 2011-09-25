#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <boost/function.hpp>
#include <boost/function_types/function_arity.hpp>
#include <boost/function_types/parameter_types.hpp>
#include <boost/interprocess/ipc/message_queue.hpp>
#include <boost/noncopyable.hpp>
#include <boost/scoped_array.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/static_assert.hpp>
#include <boost/thread.hpp>

#include <iostream>
#include <sstream>
#include <string>

// ArgType must be binary serializable
template<typename ArgType>
class ThreadPool
:
	boost::noncopyable
{
	public:

		typedef ArgType arg_type;
		typedef boost::function<void(ArgType &)> func_type;

	private:

		struct WorkerThread {
			void operator()(func_type func, char const *inputQueueName, char const *outputQueueName);
		};

		static unsigned instanceCount;

		boost::scoped_array<char const> inputQueueName;
		boost::scoped_array<char const> outputQueueName;
		boost::scoped_ptr<boost::interprocess::message_queue> inputQueue;
		boost::scoped_ptr<boost::interprocess::message_queue> outputQueue;

		boost::scoped_array<boost::scoped_ptr<boost::thread> > threads;

	public:

		ThreadPool(func_type const &func, unsigned queueSize, unsigned size = 0);
		~ThreadPool();

		void enqueue(arg_type const &arg);
		bool dequeue(arg_type *arg);

		char const *makeQueueName(char const *base, unsigned index) const;

};

template<typename ArgType>
void ThreadPool<ArgType>::WorkerThread::operator()(func_type func, char const *inputQueueName, char const *outputQueueName) {
	boost::interprocess::message_queue inputQueue(boost::interprocess::open_only, inputQueueName);
	boost::interprocess::message_queue outputQueue(boost::interprocess::open_only, outputQueueName);
	while (true) {
		unsigned priority;
		size_t size;
		arg_type arg;
		inputQueue.receive((void*)&arg, sizeof(arg_type), size, priority);
		func(arg);
		outputQueue.send(&arg, sizeof(arg_type), 0);
	}
}

template<typename ArgType>
unsigned ThreadPool<ArgType>::instanceCount = 0;

template<typename ArgType>
ThreadPool<ArgType>::ThreadPool(func_type const &func, unsigned queueSize, unsigned size) {
	unsigned index = instanceCount;
	++instanceCount;
	inputQueueName.reset(makeQueueName("ThreadPool::inputQueue", index));
	outputQueueName.reset(makeQueueName("ThreadPool::outputQueue", index));

	inputQueue.reset(new boost::interprocess::message_queue(
		boost::interprocess::create_only, inputQueueName.get(), queueSize, sizeof(arg_type)));
	outputQueue.reset(new boost::interprocess::message_queue(
		boost::interprocess::create_only, outputQueueName.get(), queueSize, sizeof(arg_type)));

	if (size == 0) {
		size = boost::thread::hardware_concurrency();
		if (size == 0) {
			size = 1;
		}
	}
	threads.reset(new boost::scoped_ptr<boost::thread>[size]);
	for (unsigned i = 0; i < size; ++i) {
		threads[i].reset(new boost::thread(WorkerThread(), func, inputQueueName.get(), outputQueueName.get()));
	}
}

template<typename ArgType>
ThreadPool<ArgType>::~ThreadPool() {
	// TODO kill all threads before removing the queues
	boost::interprocess::message_queue::remove(inputQueueName.get());
	boost::interprocess::message_queue::remove(outputQueueName.get());
}

template<typename ArgType>
void ThreadPool<ArgType>::enqueue(arg_type const &arg) {
	inputQueue->send(&arg, sizeof(arg_type), 0);
}

template<typename ArgType>
bool ThreadPool<ArgType>::dequeue(arg_type *arg) {
	unsigned priority;
	size_t size;
	return outputQueue->try_receive((void*)arg, sizeof(arg_type), size, priority);
}

template<typename ArgType>
char const *ThreadPool<ArgType>::makeQueueName(char const *base, unsigned index) const {
	std::ostringstream oss;
	oss << base << index;
	std::string qn = oss.str();
	char *buf = new char[qn.length() + 1];
	strcpy(buf, qn.c_str());
	std::cout << buf << '\n';
	return buf;
}

#endif
