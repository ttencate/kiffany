#ifndef SHADER_HPP
#define SHADER_HPP

#include "gl.h"

#include <iosfwd>
#include <string>

bool loadShaderFromFile(GLShader &shader, std::string const &fileName);
bool loadShaderFromStream(GLShader &shader, std::istream &stream);

std::string getShaderInfoLog(GLShader const &shader);
		
bool linkProgram(GLProgram &program, GLVertexShader const *vertexShader, GLFragmentShader const *fragmentShader);
		
std::string getProgramInfoLog(GLProgram const &program);

class ShaderProgram {

	GLVertexShader vertexShader;
	GLFragmentShader fragmentShader;
	GLProgram program;

	public:

		bool loadAndLink(std::string const &vertexShaderFileName, std::string const &fragmentShaderFileName);

};

#endif
