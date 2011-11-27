#include "buffer.h"

Buffer::Buffer()
:
	sizeInBytes(0)
{
}

GLBuffer const &Buffer::getGLBuffer() const {
	return buffer;
}

void Buffer::putData(unsigned size, void const *data, GLenum usage) {
	this->sizeInBytes = size;
	bindBuffer(GL_ARRAY_BUFFER, buffer);
	glBufferData(GL_ARRAY_BUFFER, size, data, usage);
}

bool Buffer::isEmpty() const {
	return sizeInBytes == 0;
}

unsigned Buffer::getSizeInBytes() const {
	return sizeInBytes;
}

void bindBuffer(GLenum target, Buffer const &buffer) {
	bindBuffer(target, buffer.getGLBuffer());
}
