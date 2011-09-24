#include "gl.h"

GLBuffer::GLBuffer() {
	glGenBuffers(1, &name);
}

GLBuffer::~GLBuffer() {
	glDeleteBuffers(1, &name);
}

GLBuffer::operator unsigned() const {
	return name;
}
