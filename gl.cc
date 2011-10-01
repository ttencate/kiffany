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

void GLBuffer::putData(unsigned size, void const *data, GLenum usage) {
	this->sizeInBytes = size;
	glBindBuffer(GL_ARRAY_BUFFER, name);
	glBufferData(GL_ARRAY_BUFFER, size, data, usage);
}

bool GLBuffer::isEmpty() const {
	return sizeInBytes == 0;
}

unsigned GLBuffer::getSizeInBytes() const {
	return sizeInBytes;
}

unsigned GLBuffer::getName() const {
	return name;
}
