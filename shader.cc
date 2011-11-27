#include "shader.h"

#include <iostream>
#include <fstream>
#include <stdexcept>
#include <vector>

std::ostream *shaderErrorStream = &std::cerr;

std::vector<std::string> loadLinesFromStream(std::istream &stream) {
	std::vector<std::string> lines;
	std::string line;
	while (std::getline(stream, line)) {
		line += '\n';
		lines.push_back(line);
	}
	return lines;
}

bool compileShaderFromStream(GLShader &shader, std::istream &stream) {
	std::vector<std::string> source = loadLinesFromStream(stream);
	shaderSource(shader, source);
	compileShader(shader);
	return getShader(shader, GL_COMPILE_STATUS);
}

bool compileShaderFromFile(GLShader &shader, std::string const &fileName) {
	std::ifstream file(fileName.c_str());
	return compileShaderFromStream(shader, file);
}

void printShaderInfoLog(GLShader const &shader, std::string const &name) {
	std::string log = getShaderInfoLog(shader);
	if (!log.empty()) {
		*shaderErrorStream
			<< "Info log of shader '" << name << "':\n"
			<< log;
	}
}

bool linkProgram(GLProgram &program, GLVertexShader const *vertexShader, GLFragmentShader const *fragmentShader) {
	if (vertexShader) {
		attachShader(program, *vertexShader);
	}
	if (fragmentShader) {
		attachShader(program, *fragmentShader);
	}
	linkProgram(program);
	return getProgram(program, GL_LINK_STATUS);
}

void printProgramInfoLog(GLProgram const &program, std::string const &name) {
	std::string log = getProgramInfoLog(program);
	if (!log.empty()) {
		*shaderErrorStream
			<< "Info log of program '" << name << "':\n"
			<< getProgramInfoLog(program);
	}
}

bool ShaderProgram::loadAndLink(std::string const &vertexShaderFileName, std::string const &fragmentShaderFileName) {
	bool success = true;
	if (!vertexShaderFileName.empty()) {
		success &= compileShaderFromFile(vertexShader, vertexShaderFileName);
		printShaderInfoLog(vertexShader, vertexShaderFileName);
	}
	if (!fragmentShaderFileName.empty()) {
		success &= compileShaderFromFile(fragmentShader, fragmentShaderFileName);
		printShaderInfoLog(fragmentShader, fragmentShaderFileName);
	}
	if (success) {
		success &= linkProgram(program,
				vertexShaderFileName.empty() ? NULL : &vertexShader,
				fragmentShaderFileName.empty() ? NULL : &fragmentShader);
		validateProgram(program);
		printProgramInfoLog(program,
				(vertexShaderFileName.empty() ? "[default]" : vertexShaderFileName) + ", " +
				(fragmentShaderFileName.empty() ? "[default]" : fragmentShaderFileName));
	}
	uniforms.clear();
	return success;
}

GLUniform ShaderProgram::getUniform(std::string const &name) const {
	UniformMap::const_iterator i = uniforms.find(name);
	if (i != uniforms.end()) {
		return i->second;
	}
	GLUniform uniform = getUniformLocation(program, name);
	if (!uniform.isValid()) {
		*shaderErrorStream << "Invalid uniform: '" << name << "' (optimized out?)\n";
	}
	uniforms[name] = uniform;
	return uniform;
}
