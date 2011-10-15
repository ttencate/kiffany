#include "threadpool.h"

#include <iostream>

Semaphore::Semaphore(unsigned initialCount)
:
	count(initialCount)
{
}

void Semaphore::wait() {
	boost::unique_lock<boost::mutex> lock(mutex);
	while (count == 0) {
		condition_variable.wait(lock);
	}
	--count;
}

bool Semaphore::tryWait() {
	boost::unique_lock<boost::mutex> lock(mutex);
	if (count > 0) {
		--count;
		return true;
	} else {
		return false;
	}
}

void Semaphore::signal() {
	boost::unique_lock<boost::mutex> lock(mutex);
	++count;
	condition_variable.notify_one();
}

WorkQueue::WorkQueue(unsigned maxSize)
:
	maxSize(maxSize),
	semaphore(maxSize == 0 ? std::numeric_limits<unsigned>::max() : maxSize)
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
	semaphore.wait();
	queue.post(boost::bind(&WorkQueue::runAndSignal, this, worker));
}

bool WorkQueue::tryPost(Worker worker) {
	if (semaphore.tryWait()) {
		queue.post(boost::bind(&WorkQueue::runAndSignal, this, worker));
		return true;
	} else {
		return false;
	}
}

void WorkQueue::runAndSignal(Worker worker) {
	semaphore.signal();
	worker();
}

ThreadPool::ThreadPool(unsigned maxInputQueueSize, unsigned maxOutputQueueSize, unsigned numThreads)
:
	numThreads(numThreads == 0 ? defaultNumThreads() : numThreads),
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
	runFinalizers();
	// Do not delete the queues until all threads have finished.
	for (unsigned i = 0; i < numThreads; ++i) {
		threads[i]->join();
		runFinalizers();
	}
}

void ThreadPool::enqueue(Worker worker) {
	inputQueue.post(boost::bind(&ThreadPool::work, this, worker));
}

bool ThreadPool::tryEnqueue(Worker worker) {
	return inputQueue.tryPost(boost::bind(&ThreadPool::work, this, worker));
}

void ThreadPool::runFinalizers() {
	outputQueue.run();
	outputQueue.reset();
}

unsigned ThreadPool::defaultNumThreads() {
	unsigned numThreads = boost::thread::hardware_concurrency();
	if (numThreads == 0) {
		numThreads = 1;
	}
	return numThreads;
}

void ThreadPool::work(Worker worker) {
	Finalizer finalizer = worker();
	outputQueue.post(boost::bind(&ThreadPool::finalize, this, finalizer));
}

// This wrapper is not currently needed, but it might be useful later for e.g. stats counting.
void ThreadPool::finalize(Finalizer finalizer) {
	finalizer();
}
