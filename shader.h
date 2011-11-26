#ifndef SHADER_HPP
#define SHADER_HPP

#include "gl.h"

#include <iosfwd>
#include <string>

template<GLenum Type>
class GLShader {
	
	GLuint name;
	
	public:
		
		GLShader();
		~GLShader();

		bool loadFromFile(std::string const &fileName);
		bool loadFromStream(std::istream &stream);
		
		GLuint getName() const { return name; }
};

template<GLenum Type>
std::string getShaderInfoLog(GLShader<Type> const &shader);

typedef GLShader<GL_VERTEX_SHADER> GLVertexShader;
typedef GLShader<GL_FRAGMENT_SHADER> GLFragmentShader;

class GLProgram {
	
	GLuint name;
	
	public:
		
		GLProgram();
		~GLProgram();

		GLuint getName() const { return name; }
};

bool linkProgram(GLProgram &program, GLVertexShader const *vertexShader, GLFragmentShader const *fragmentShader);
		
std::string getProgramInfoLog(GLProgram const &program);

#endif
