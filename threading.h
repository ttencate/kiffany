#ifndef THREADING_H
#define THREADING_H

#include <boost/noncopyable.hpp>
#include <boost/thread.hpp>

class Semaphore
:
	boost::noncopyable
{
	unsigned count;
	boost::mutex mutex;
	boost::condition_variable condition_variable;

	public:

		Semaphore(unsigned initialCount);

		void wait();
		bool tryWait();
		void signal();

};

#endif
