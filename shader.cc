#include "shader.h"

#include <iostream>
#include <fstream>
#include <stdexcept>
#include <vector>

template<GLenum Type>
GLShader<Type>::GLShader()
:
	name(glCreateShader(Type))
{
}

template<GLenum Type>
GLShader<Type>::~GLShader() {
	glDeleteShader(name);
}

template<GLenum Type>
bool GLShader<Type>::loadFromFile(std::string const &fileName) {
	std::ifstream file(fileName.c_str());
	return loadFromStream(file);
}

template<GLenum Type>
bool GLShader<Type>::loadFromStream(std::istream &stream) {
	std::vector<std::string> lines;
	std::string line;
	while (std::getline(stream, line)) {
		line += '\n';
		lines.push_back(line);
	}

	std::vector<char const*> clines;
	for (std::vector<std::string>::const_iterator i = lines.begin(); i != lines.end(); ++i) {
		clines.push_back(i->c_str());
	}

	glShaderSource(name, lines.size(), &clines[0], 0);
	glCompileShader(name);
	
	int success;
	glGetShaderiv(name, GL_COMPILE_STATUS, &success);
	return success != 0;
}

template<GLenum Type>
std::string getShaderInfoLog(GLShader<Type> const &shader) {
	int logLength;
	glGetShaderiv(shader.getName(), GL_INFO_LOG_LENGTH, &logLength);
	
	std::string log;
	if (logLength > 0) {
		std::vector<char> buffer(logLength);
		glGetShaderInfoLog(shader.getName(), logLength, &logLength, &buffer[0]);
		log = &buffer[0];
	}
	
	return log;
}

template class GLShader<GL_VERTEX_SHADER>;
template class GLShader<GL_FRAGMENT_SHADER>;

template<>
std::string getShaderInfoLog(GLShader<GL_VERTEX_SHADER> const &shader);
template<>
std::string getShaderInfoLog(GLShader<GL_FRAGMENT_SHADER> const &shader);

GLProgram::GLProgram()
:
	name(glCreateProgram())
{
}

bool linkProgram(GLProgram &program, GLVertexShader const *vertexShader, GLFragmentShader const *fragmentShader) {
	if (vertexShader) {
		glAttachShader(program.getName(), vertexShader->getName());
	}
	if (fragmentShader) {
		glAttachShader(program.getName(), fragmentShader->getName());
	}
	
	glLinkProgram(program.getName());
	
	int success;
	glGetProgramiv(program.getName(), GL_LINK_STATUS, &success);
	return success != 0;
}

GLProgram::~GLProgram() {
	glDeleteProgram(name);
}

std::string getProgramInfoLog(GLProgram const &program) {
	int logLength;
	glGetProgramiv(program.getName(), GL_INFO_LOG_LENGTH, &logLength);
	
	std::string log;
	if (logLength > 0) {
		char *buffer = new char[logLength];
		glGetProgramInfoLog(program.getName(), logLength, &logLength, buffer);
		log = buffer;
		delete[] buffer;
	}
	
	return log;
}
