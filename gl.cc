#include "gl.h"

#include <boost/static_assert.hpp>

GLBuffer::GLBuffer() {
	glGenBuffers(1, &name);
}

GLBuffer::~GLBuffer() {
	glDeleteBuffers(1, &name);
}

void bindBuffer(GLenum target, GLBuffer const &buffer) {
	glBindBuffer(target, buffer.getName());
}

GLTexture::GLTexture() {
	glGenTextures(1, &name);
}

GLTexture::~GLTexture() {
	glDeleteTextures(1, &name);
}

void activeTexture(unsigned texture) {
	glActiveTexture(GL_TEXTURE0 + texture);
}

void bindTexture(GLenum target, GLTexture const &texture) {
	glBindTexture(target, texture.getName());
}

GLShader::GLShader(GLenum type)
:
	name(glCreateShader(type))
{
}

GLShader::~GLShader() {
	glDeleteShader(name);
}

void shaderSource(GLShader &shader, std::vector<std::string> const &source) {
	std::vector<char const*> clines;
	for (std::vector<std::string>::const_iterator i = source.begin(); i != source.end(); ++i) {
		clines.push_back(i->c_str());
	}

	glShaderSource(shader.getName(), clines.size(), &clines[0], 0);
}

void compileShader(GLShader &shader) {
	glCompileShader(shader.getName());
}

int getShader(GLShader const &shader, GLenum pname) {
	GLint params;
	glGetShaderiv(shader.getName(), pname, &params);
	return params;
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

GLVertexShader::GLVertexShader()
:
	GLShader(GL_VERTEX_SHADER)
{
}

GLFragmentShader::GLFragmentShader()
:
	GLShader(GL_FRAGMENT_SHADER)
{
}

GLProgram::GLProgram()
:
	name(glCreateProgram())
{
}

GLProgram::~GLProgram() {
	glDeleteProgram(name);
}

void attachShader(GLProgram &program, GLShader const &shader) {
	glAttachShader(program.getName(), shader.getName());
}

void linkProgram(GLProgram &program) {
	glLinkProgram(program.getName());
}

int getProgram(GLProgram const &program, GLenum pname) {
	GLint params;
	glGetProgramiv(program.getName(), pname, &params);
	return params;
}

void validateProgram(GLProgram const &program) {
	glValidateProgram(program.getName());
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

void useProgram(GLProgram const &program) {
	glUseProgram(program.getName());
}

void useFixedProcessing() {
	glUseProgram(0);
}

void bindFragDataLocation(GLProgram &program, unsigned number, std::string const &name) {
	glBindFragDataLocation(program.getName(), number, name.c_str());
}

GLUniform::GLUniform(GLint location)
:
	location(location)
{
}

GLUniform getUniformLocation(GLProgram const &program, std::string const &name) {
	GLint location = glGetUniformLocation(program.getName(), name.c_str());
	return GLUniform(location);
}

void uniform(GLUniform const &uniform, float v) {
	glUniform1f(uniform.getLocation(), v);
}

void uniform(GLUniform const &uniform, glm::vec2 v) {
	glUniform2f(uniform.getLocation(), v.x, v.y);
}

void uniform(GLUniform const &uniform, glm::vec3 v) {
	glUniform3f(uniform.getLocation(), v.x, v.y, v.z);
}

void uniform(GLUniform const &uniform, glm::vec4 v) {
	glUniform4f(uniform.getLocation(), v.x, v.y, v.z, v.w);
}

void uniform(GLUniform const &uniform, int v) {
	glUniform1i(uniform.getLocation(), v);
}

void uniform(GLUniform const &uniform, glm::ivec2 v) {
	glUniform2i(uniform.getLocation(), v.x, v.y);
}

void uniform(GLUniform const &uniform, glm::ivec3 v) {
	glUniform3i(uniform.getLocation(), v.x, v.y, v.z);
}

void uniform(GLUniform const &uniform, glm::ivec4 v) {
	glUniform4i(uniform.getLocation(), v.x, v.y, v.z, v.w);
}

void uniform(GLUniform const &uniform, std::vector<float> const &v) {
	glUniform1fv(uniform.getLocation(), v.size(), &v[0]);
}

void uniform(GLUniform const &uniform, std::vector<glm::vec2> const &v) {
	BOOST_STATIC_ASSERT(sizeof(glm::vec2) == 2 * sizeof(float));
	glUniform2fv(uniform.getLocation(), v.size(), (float const *)&v[0]);
}

void uniform(GLUniform const &uniform, std::vector<glm::vec3> const &v) {
	BOOST_STATIC_ASSERT(sizeof(glm::vec3) == 3 * sizeof(float));
	glUniform3fv(uniform.getLocation(), v.size(), (float const *)&v[0]);
}

void uniform(GLUniform const &uniform, std::vector<glm::vec4> const &v) {
	BOOST_STATIC_ASSERT(sizeof(glm::vec4) == 4 * sizeof(float));
	glUniform4fv(uniform.getLocation(), v.size(), (float const *)&v[0]);
}

void uniform(GLUniform const &uniform, std::vector<int> const &v) {
	glUniform1iv(uniform.getLocation(), v.size(), &v[0]);
}

void uniform(GLUniform const &uniform, std::vector<glm::ivec2> const &v) {
	BOOST_STATIC_ASSERT(sizeof(glm::ivec2) == 2 * sizeof(int));
	glUniform2iv(uniform.getLocation(), v.size(), (int const *)&v[0]);
}

void uniform(GLUniform const &uniform, std::vector<glm::ivec3> const &v) {
	BOOST_STATIC_ASSERT(sizeof(glm::ivec3) == 3 * sizeof(int));
	glUniform3iv(uniform.getLocation(), v.size(), (int const *)&v[0]);
}

void uniform(GLUniform const &uniform, std::vector<glm::ivec4> const &v) {
	BOOST_STATIC_ASSERT(sizeof(glm::ivec4) == 4 * sizeof(int));
	glUniform4iv(uniform.getLocation(), v.size(), (int const *)&v[0]);
}
