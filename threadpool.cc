#include "threadpool.h"

#include <iostream>

WorkQueue::WorkQueue(unsigned maxSize)
:
	maxSize(maxSize),
	semaphore(maxSize == 0 ? std::numeric_limits<unsigned>::max() : maxSize)
{
}

void WorkQueue::post(Worker worker, Priority priority) {
	semaphore.wait();
	doPost(worker, priority);
}

bool WorkQueue::tryPost(Worker worker, Priority priority) {
	if (semaphore.tryWait()) {
		doPost(worker, priority);
		return true;
	} else {
		return false;
	}
}

void WorkQueue::runOne() {
	Worker worker;
	{
		boost::unique_lock<boost::mutex> lock(queueMutex);
		while (queue.empty()) {
			conditionVariable.wait(lock);
		}
		worker = queue.front();
		queue.pop_front();
	}
	semaphore.signal();
	worker();
}

void WorkQueue::runAll() {
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

void WorkQueue::doPost(Worker worker, Priority priority) {
	{
		boost::unique_lock<boost::mutex> lock(queueMutex);
		queue.insert(worker, priority);
	}
	conditionVariable.notify_one();
}

ThreadPool::ThreadPool(unsigned maxInputQueueSize, unsigned maxOutputQueueSize, unsigned numThreads)
:
	numThreads(numThreads == 0 ? defaultNumThreads() : numThreads),
	inputQueue(maxInputQueueSize),
	outputQueue(maxOutputQueueSize),
	threads(new boost::scoped_ptr<boost::thread>[numThreads])
{
	for (unsigned i = 0; i < numThreads; ++i) {
		threads[i].reset(new boost::thread(boost::bind(&ThreadPool::loop, this)));
	}
}

ThreadPool::~ThreadPool() {
	for (unsigned i = 0; i < numThreads; ++i) {
		threads[i]->interrupt();
	}
	// Do not delete the queues until all threads have finished.
	// Ensure that there is enough space in the output queue for them.
	runFinalizers();
	for (unsigned i = 0; i < numThreads; ++i) {
		threads[i]->join();
		runFinalizers();
	}
}

void ThreadPool::enqueue(Worker worker, Priority priority) {
	inputQueue.post(boost::bind(&ThreadPool::work, this, worker), priority);
}

bool ThreadPool::tryEnqueue(Worker worker, Priority priority) {
	return inputQueue.tryPost(boost::bind(&ThreadPool::work, this, worker), priority);
}

void ThreadPool::runFinalizers() {
	outputQueue.runAll();
}

unsigned ThreadPool::defaultNumThreads() {
	unsigned numThreads = boost::thread::hardware_concurrency();
	if (numThreads == 0) {
		numThreads = 1;
	}
	return numThreads;
}

void ThreadPool::loop() {
	while (true) {
		inputQueue.runOne();
	}
}

void ThreadPool::work(Worker worker) {
	Finalizer finalizer = worker();
	outputQueue.post(boost::bind(&ThreadPool::finalize, this, finalizer));
}

// This wrapper is not currently needed, but it might be useful later for e.g. stats counting.
void ThreadPool::finalize(Finalizer finalizer) {
	finalizer();
}
