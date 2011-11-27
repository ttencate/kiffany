#ifndef BUFFER_H
#define BUFFER_H

#include "gl.h"

class Buffer {

	GLBuffer buffer;

	unsigned sizeInBytes;

	public:
		
		Buffer();

		// TODO this lacks elegance; wrap things that would need binding
		GLBuffer const &getGLBuffer() const;

		void putData(unsigned size, void const *data, GLenum usage);

		bool isEmpty() const;
		unsigned getSizeInBytes() const;
};

void bindBuffer(GLenum target, Buffer const &buffer);

#endif
