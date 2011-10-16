#include "threading.h"

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
