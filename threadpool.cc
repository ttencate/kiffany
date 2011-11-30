#include "threadpool.h"

WorkQueue::WorkQueue(unsigned maxSize)
:
	maxSize(maxSize),
	semaphore(maxSize == 0 ? std::numeric_limits<unsigned>::max() : maxSize)
{
}

void WorkQueue::post(Worker worker) {
	semaphore.wait();
	doPost(worker);
}

bool WorkQueue::tryPost(Worker worker) {
	if (semaphore.tryWait()) {
		doPost(worker);
		return true;
	} else {
		return false;
	}
}

void WorkQueue::runOne() {
	boost::unique_lock<boost::mutex> lock(queueMutex);
	while (deque.empty()) {
		conditionVariable.wait(lock);
	}
	Worker worker(deque.front());
	deque.pop_front();
	lock.unlock();

	semaphore.signal();
	worker();
}

void WorkQueue::runAll() {
	Deque all;
	{
		boost::unique_lock<boost::mutex> lock(queueMutex);
		std::swap(deque, all);
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

unsigned WorkQueue::getSize() const {
	// TODO In case of contention, shared_lock with shared_mutex can be used here.
	boost::unique_lock<boost::mutex> lock(queueMutex);
	return deque.size();
}

unsigned WorkQueue::getMaxSize() const {
	return maxSize;
}

void WorkQueue::doPost(Worker worker) {
	{
		boost::unique_lock<boost::mutex> lock(queueMutex);
		deque.push_back(worker);
	}
	conditionVariable.notify_one();
}

ThreadPool::ThreadPool(unsigned maxQueueSize, unsigned numThreads)
:
	numThreads(numThreads == 0 ? defaultNumThreads() : numThreads),
	queue(maxQueueSize),
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
	// Do not delete the queue until all threads have finished.
	for (unsigned i = 0; i < numThreads; ++i) {
		threads[i]->join();
	}
}

void ThreadPool::enqueue(Worker worker) {
	queue.post(worker);
}

bool ThreadPool::tryEnqueue(Worker worker) {
	return queue.tryPost(worker);
}

unsigned ThreadPool::getQueueSize() const {
	return queue.getSize();
}

unsigned ThreadPool::getMaxQueueSize() const {
	return queue.getMaxSize();
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
		queue.runOne();
	}
}
