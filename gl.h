#ifndef GL_H
#define GL_H

#include <GL/glew.h>
#include <GL/glfw.h>

#include <boost/noncopyable.hpp>

// TODO refactor this to become a trivial resource-managing wrapper like the others
class GLBuffer : boost::noncopyable {

	GLuint name;
	unsigned sizeInBytes;

	public:

		GLBuffer();
		~GLBuffer();

		void putData(unsigned size, void const *data, GLenum usage);

		bool isEmpty() const;
		unsigned getSizeInBytes() const;

		GLuint getName() const;
};

class GLTexture : boost::noncopyable {

	GLuint name;

	public:

		GLTexture();
		~GLTexture();

		GLuint getName() const;
};

class GLShader : boost::noncopyable {

	GLuint name;
	
	public:
		
		~GLShader();

		GLuint getName() const { return name; }

	protected:

		GLShader(GLenum type);
};

class GLVertexShader : public GLShader {

	public:

		GLVertexShader();
};

class GLFragmentShader : public GLShader {

	public:

		GLFragmentShader();
};

class GLProgram : boost::noncopyable {
	
	GLuint name;
	
	public:
		
		GLProgram();
		~GLProgram();

		GLuint getName() const { return name; }
};

#endif
