#include "threadpool.h"

WorkQueue::WorkQueue(unsigned maxSize)
:
	maxSize(maxSize),
	size(0)
{
}

boost::asio::io_service::work *WorkQueue::createWork() {
	return new boost::asio::io_service::work(queue);
}

void WorkQueue::run() {
	queue.run();
}

void WorkQueue::reset() {
	queue.reset();
}

void WorkQueue::stop() {
	queue.stop();
}

void WorkQueue::post(Worker worker) {
	queue.post(boost::bind(&WorkQueue::runAndDecrementSize, this, worker));
}

void WorkQueue::runAndDecrementSize(Worker worker) {
	worker();
}

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

ThreadPool::ThreadPool(unsigned maxInputQueueSize, unsigned maxOutputQueueSize, unsigned requestedNumThreads)
:
	numThreads(computeNumThreads(requestedNumThreads)),
	inputQueue(maxInputQueueSize),
	outputQueue(maxOutputQueueSize),
	works(new boost::scoped_ptr<boost::asio::io_service::work>[numThreads]),
	threads(new boost::scoped_ptr<boost::thread>[numThreads])
{
	for (unsigned i = 0; i < numThreads; ++i) {
		works[i].reset(inputQueue.createWork());
		threads[i].reset(new boost::thread(boost::bind(&WorkQueue::run, &inputQueue)));
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
