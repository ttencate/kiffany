#ifndef SHADER_HPP
#define SHADER_HPP

#include "gl.h"

#include <iosfwd>
#include <map>
#include <string>

std::vector<std::string> loadLinesFromStream(std::istream &stream);

bool compileShaderFromStream(GLShader &shader, std::istream &stream);
bool compileShaderFromFile(GLShader &shader, std::string const &fileName);

void printShaderInfoLog(GLShader const &shader, std::string const &fileName);

bool linkProgram(GLProgram &program, GLVertexShader const *vertexShader, GLFragmentShader const *fragmentShader);
void printProgramInfoLog(GLProgram const &program, std::string const &name);
		
class ShaderProgram {

	friend void useProgram(ShaderProgram const &);

	GLVertexShader vertexShader;
	GLFragmentShader fragmentShader;
	GLProgram program;

	typedef std::map<std::string, GLUniform> UniformMap;
	UniformMap mutable uniforms;

	public:

		bool loadAndLink(std::string const &vertexShaderFileName, std::string const &fragmentShaderFileName);

		GLUniform getUniform(std::string const &name) const;

		template<typename T>
		void setUniform(std::string const &name, T const &value) {
			uniform(getUniform(name), value);
		}

};

void useProgram(ShaderProgram const &shaderProgram);

#endif
