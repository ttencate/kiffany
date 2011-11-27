#include "shader.h"

#include <iostream>
#include <fstream>
#include <stdexcept>
#include <vector>

std::ostream *shaderErrorStream = &std::cerr;

bool loadShaderFromFile(GLShader &shader, std::string const &fileName) {
	std::ifstream file(fileName.c_str());
	return loadShaderFromStream(shader, file);
}

bool loadShaderFromStream(GLShader &shader, std::istream &stream) {
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

	glShaderSource(shader.getName(), lines.size(), &clines[0], 0);
	glCompileShader(shader.getName());
	
	int success;
	glGetShaderiv(shader.getName(), GL_COMPILE_STATUS, &success);
	return success != 0;
}

std::string getShaderInfoLog(GLShader const &shader) {
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

bool ShaderProgram::loadAndLink(std::string const &vertexShaderFileName, std::string const &fragmentShaderFileName) {
	bool success = true;
	if (!vertexShaderFileName.empty()) {
		success &= loadShaderFromFile(vertexShader, vertexShaderFileName);
		std::string log = getShaderInfoLog(vertexShader);
		if (!log.empty()) {
			*shaderErrorStream
				<< "Compilation of vertex shader '" << vertexShaderFileName << "':\n"
				<< getShaderInfoLog(vertexShader);
		}
	}
	if (!fragmentShaderFileName.empty()) {
		success &= loadShaderFromFile(fragmentShader, fragmentShaderFileName);
		std::string log = getShaderInfoLog(fragmentShader);
		if (!log.empty()) {
			*shaderErrorStream
				<< "Compilation of fragment shader '" << fragmentShaderFileName << "':\n"
				<< getShaderInfoLog(fragmentShader);
		}
	}
	if (success) {
		success &= linkProgram(program,
				vertexShaderFileName.empty() ? NULL : &vertexShader,
				fragmentShaderFileName.empty() ? NULL : &fragmentShader);
		std::string log = getProgramInfoLog(program);
		if (!log.empty()) {
			*shaderErrorStream
				<< "Linking of program with vertex shader '" << vertexShaderFileName << "', "
				<< "fragment shader '" << fragmentShaderFileName << "':\n"
				<< getProgramInfoLog(program);
		}
		uniforms.clear();
	}
	return success;
}

GLint ShaderProgram::getUniformLocation(std::string const &name) const {
	UniformMap::const_iterator i = uniforms.find(name);
	if (i != uniforms.end()) {
		return i->second;
	}
	GLint uniform = glGetUniformLocation(program.getName(), name.c_str());
	if (uniform == -1) {
		*shaderErrorStream << "Uniform not found: '" << name << "'\n";
	} else {
		uniforms[name] = uniform;
	}
	return uniform;
}
