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
	unsigned sizeInBytes;

	public:

		GLBuffer();
		~GLBuffer();

		void putData(unsigned size, void const *data, GLenum usage);

		bool isEmpty() const;
		unsigned getSizeInBytes() const;

		unsigned getName() const;

};

#endif
