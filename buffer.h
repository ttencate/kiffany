#ifndef BUFFER_H
#define BUFFER_H

#include <boost/noncopyable.hpp>

class Buffer
:
	boost::noncopyable
{

	unsigned name;

	public:

		Buffer();
		~Buffer();

		operator unsigned() const; // TODO stop being lazy and make into a function

};

#endif
