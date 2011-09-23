#include "buffer.h"

#include <GL/glew.h>

Buffer::Buffer() {
	glGenBuffers(1, &name);
}

Buffer::~Buffer() {
	glDeleteBuffers(1, &name);
}

Buffer::operator unsigned() const {
	return name;
}
