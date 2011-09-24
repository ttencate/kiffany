#include "gl.h"

GLBuffer::GLBuffer()
:
	sizeInBytes(0)
{
	glGenBuffers(1, &name);
}

GLBuffer::~GLBuffer() {
	glDeleteBuffers(1, &name);
}

void GLBuffer::putData(unsigned size, void *data, GLenum usage) {
	this->sizeInBytes = size;
	glBindBuffer(GL_ARRAY_BUFFER, name);
	glBufferData(GL_ARRAY_BUFFER, size, data, usage);
}

unsigned GLBuffer::getSizeInBytes() const {
	return sizeInBytes;
}

GLBuffer::operator unsigned() const {
	return name;
}
