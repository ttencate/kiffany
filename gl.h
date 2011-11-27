#ifndef GL_H
#define GL_H

#include "maths.h"

#include <GL/glew.h>

#include <boost/noncopyable.hpp>

#include <string>
#include <vector>

/* Everything in this file is a thin, simple wrapper around OpenGL stuff.
 * All classes are direct equivalents of OpenGL objects,
 * and all functions are direct counterparts of OpenGL functions.
 *
 * Basically, the only things that happen here are resource management
 * and type conversions.
 */

class GLBuffer : boost::noncopyable {

	GLuint name;

	public:

		GLBuffer();
		~GLBuffer();

		GLuint getName() const { return name; }
};

void bindBuffer(GLenum target, GLBuffer const &buffer);

class GLTexture : boost::noncopyable {

	GLuint name;

	public:

		GLTexture();
		~GLTexture();

		GLuint getName() const { return name; }
};

void activeTexture(unsigned texture);
void bindTexture(GLenum target, GLTexture const &texture);

class GLShader : boost::noncopyable {

	GLuint name;
	
	public:
		
		~GLShader();

		GLuint getName() const { return name; }

	protected:

		GLShader(GLenum type);
};

void shaderSource(GLShader &shader, std::vector<std::string> const &source);
void compileShader(GLShader &shader);
int getShader(GLShader const &shader, GLenum pname);
std::string getShaderInfoLog(GLShader const &shader);
		
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

void attachShader(GLProgram &program, GLShader const &shader);
void linkProgram(GLProgram &program);
int getProgram(GLProgram const &program, GLenum pname);
void validateProgram(GLProgram const &program);
std::string getProgramInfoLog(GLProgram const &program);
void useProgram(GLProgram const &program);
void useFixedProcessing();
void bindFragDataLocation(GLProgram &program, unsigned number, std::string const &name);

class GLUniform {

	GLint location;

	public:

		GLUniform(GLint location = -1);

		GLint getLocation() const { return location; }
		bool isValid() const { return location != -1; }
};

GLUniform getUniformLocation(GLProgram const &program, std::string const &name);
void uniform(GLUniform const &uniform, float v);
void uniform(GLUniform const &uniform, glm::vec2 v);
void uniform(GLUniform const &uniform, glm::vec3 v);
void uniform(GLUniform const &uniform, glm::vec4 v);
void uniform(GLUniform const &uniform, int v);
void uniform(GLUniform const &uniform, glm::ivec2 v);
void uniform(GLUniform const &uniform, glm::ivec3 v);
void uniform(GLUniform const &uniform, glm::ivec4 v);
void uniform(GLUniform const &uniform, std::vector<float> const &v);
void uniform(GLUniform const &uniform, std::vector<glm::vec2> const &v);
void uniform(GLUniform const &uniform, std::vector<glm::vec3> const &v);
void uniform(GLUniform const &uniform, std::vector<glm::vec4> const &v);
void uniform(GLUniform const &uniform, std::vector<int> const &v);
void uniform(GLUniform const &uniform, std::vector<glm::ivec2> const &v);
void uniform(GLUniform const &uniform, std::vector<glm::ivec3> const &v);
void uniform(GLUniform const &uniform, std::vector<glm::ivec4> const &v);

#endif
