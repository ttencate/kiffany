#ifndef SHADER_HPP
#define SHADER_HPP

#include "gl.h"

#include <iosfwd>
#include <map>
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

	typedef std::map<std::string, GLint> UniformMap;
	UniformMap mutable uniforms;

	public:

		bool loadAndLink(std::string const &vertexShaderFileName, std::string const &fragmentShaderFileName);

		GLint getUniformLocation(std::string const &name) const;

		template<typename T>
		void setUniform(std::string const &name, T const &value) {
			GLint uniform = getUniformLocation(name);
			glUniform(uniform, value);
		}

};

#endif
