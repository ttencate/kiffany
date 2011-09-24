#ifndef GL_H
#define GL_H

#include <GL/glew.h>
#include <GL/glfw.h>

#include <boost/noncopyable.hpp>

class GLBuffer
:
	boost::noncopyable
{

	unsigned name;

	public:

		GLBuffer();
		~GLBuffer();

		operator unsigned() const; // TODO stop being lazy and make into a function

};

#endif