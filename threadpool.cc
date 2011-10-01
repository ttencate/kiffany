#include "threadpool.h"

unsigned computeSize(int requestedSize) {
	int size;
	if (requestedSize == 0) {
		size = boost::thread::hardware_concurrency() - 1;
		if (size == 0) {
			size = 1;
		}
	} else {
		size = requestedSize;
	}
	return size;
}

ThreadPool::ThreadPool(unsigned requestedSize)
:
	size(computeSize(requestedSize))
{
	works.reset(new boost::scoped_ptr<boost::asio::io_service::work>[size]);
	threads.reset(new boost::scoped_ptr<boost::thread>[size]);
	for (unsigned i = 0; i < size; ++i) {
		works[i].reset(new boost::asio::io_service::work(inputQueue));
		threads[i].reset(new boost::thread(boost::bind(&boost::asio::io_service::run, &inputQueue)));
	}
}

ThreadPool::~ThreadPool() {
	inputQueue.stop();
	// Do not delete the queues until all threads have finished.
	for (unsigned i = 0; i < size; ++i) {
		threads[i]->join();
	}
}

void ThreadPool::enqueue(Worker worker, Finalizer finalizer) {
	inputQueue.post(boost::bind(&ThreadPool::work, this, worker, finalizer));
}

void ThreadPool::runFinalizers() {
	outputQueue.run();
	outputQueue.reset();
}

void ThreadPool::work(Worker worker, Finalizer finalizer) {
	worker();
	outputQueue.post(boost::bind(&ThreadPool::finalize, this, finalizer));
}

// This wrapper is not currently needed, but it might be useful later for e.g. stats counting.
void ThreadPool::finalize(Finalizer finalizer) {
	finalizer();
}
