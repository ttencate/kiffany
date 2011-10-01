#include "threadpool.h"

unsigned computeNumThreads(int requestedNumThreads) {
	int numThreads;
	if (requestedNumThreads == 0) {
		numThreads = boost::thread::hardware_concurrency() - 1;
		if (numThreads == 0) {
			numThreads = 1;
		}
	} else {
		numThreads = requestedNumThreads;
	}
	return numThreads;
}

ThreadPool::ThreadPool(unsigned requestedNumThreads)
:
	numThreads(computeNumThreads(requestedNumThreads)),
	works(new boost::scoped_ptr<boost::asio::io_service::work>[numThreads]),
	threads(new boost::scoped_ptr<boost::thread>[numThreads])
{
	for (unsigned i = 0; i < numThreads; ++i) {
		works[i].reset(new boost::asio::io_service::work(inputQueue));
		threads[i].reset(new boost::thread(boost::bind(&boost::asio::io_service::run, &inputQueue)));
	}
}

ThreadPool::~ThreadPool() {
	inputQueue.stop();
	// Do not delete the queues until all threads have finished.
	for (unsigned i = 0; i < numThreads; ++i) {
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
