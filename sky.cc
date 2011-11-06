#include "sky.h"

Sky::Sky()
:
	size(16),
	textureImage(new unsigned char[size * size * 3])
{
	int const v[] = {
		-1, -1, -1,
		-1,  1, -1,
		-1,  1,  1,
		-1, -1,  1,

		 1, -1, -1,
		 1, -1,  1,
		 1,  1,  1,
		 1,  1, -1,

		-1, -1, -1,
		-1, -1,  1,
		 1, -1,  1,
		 1, -1, -1,

		-1,  1, -1,
		 1,  1, -1,
		 1,  1,  1,
		-1,  1,  1,

		-1, -1, -1,
		 1, -1, -1,
		 1,  1, -1,
		-1,  1, -1,
		
		-1, -1,  1,
		-1,  1,  1,
		 1,  1,  1,
		 1, -1,  1
	};
	vertices.putData(sizeof(v), v, GL_STATIC_DRAW);

	glBindTexture(GL_TEXTURE_CUBE_MAP, texture.getName());
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	generateFaces();
}

void Sky::generateFace(GLenum face) {
	unsigned char nx[] = { 0x0, 0x7F, 0x7F };
	unsigned char px[] = { 0xFF, 0x7F, 0x7F };
	unsigned char ny[] = { 0x7F, 0x0, 0x7F };
	unsigned char py[] = { 0x7F, 0xFF, 0x7F };
	unsigned char nz[] = { 0x7F, 0x7F, 0x0 };
	unsigned char pz[] = { 0x7F, 0x7F, 0xFF };
	unsigned char *p = 0;
	switch (face) {
		case GL_TEXTURE_CUBE_MAP_NEGATIVE_X: p = nx; break;
		case GL_TEXTURE_CUBE_MAP_POSITIVE_X: p = px; break;
		case GL_TEXTURE_CUBE_MAP_NEGATIVE_Y: p = ny; break;
		case GL_TEXTURE_CUBE_MAP_POSITIVE_Y: p = py; break;
		case GL_TEXTURE_CUBE_MAP_NEGATIVE_Z: p = nz; break;
		case GL_TEXTURE_CUBE_MAP_POSITIVE_Z: p = pz; break;
	}
	for (unsigned i = 0; i < size * size * 3;) {
		textureImage[i++] = p[0];
		textureImage[i++] = p[1];
		textureImage[i++] = p[2];
	}
	glTexImage2D(face, 0, GL_RGB, size, size, 0, GL_RGB, GL_UNSIGNED_BYTE, textureImage.get());
}

void Sky::generateFaces() {
	glBindTexture(GL_TEXTURE_CUBE_MAP, texture.getName());
	generateFace(GL_TEXTURE_CUBE_MAP_NEGATIVE_X);
	generateFace(GL_TEXTURE_CUBE_MAP_POSITIVE_X);
	generateFace(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y);
	generateFace(GL_TEXTURE_CUBE_MAP_POSITIVE_Y);
	generateFace(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z);
	generateFace(GL_TEXTURE_CUBE_MAP_POSITIVE_Z);
}

void Sky::update(float dt) {
}

void Sky::render() {
	glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);
	glDepthMask(GL_FALSE);
	glEnable(GL_TEXTURE_CUBE_MAP);

	glEnableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);	
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	glBindBuffer(GL_ARRAY_BUFFER, vertices.getName());
	glVertexPointer(3, GL_INT, 0, 0);
	glTexCoordPointer(3, GL_INT, 0, 0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, texture.getName());
	glDrawArrays(GL_QUADS, 0, vertices.getSizeInBytes() / sizeof(int) / 3);

	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisable(GL_TEXTURE_CUBE_MAP);
	glDepthMask(GL_TRUE);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
}
