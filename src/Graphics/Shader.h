#pragma once
#ifndef _SHADER_H_
#define _SHADER_H_

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "Utils/UtilFunctions.h"

class Shader
{
public:
	Shader(const std::string& filePath);
	~Shader();

	void bind() const;
	void unBind() const;

	// Setting uniforms...
	void setUniform1i(const GLchar* name, GLuint val) const;
	void setUniform1f(const GLchar* name, GLfloat val) const;

private:
	enum class ShaderErrorType
	{
		vertexShader = 0,
		fragmentShader,
	};

	GLint getUniformLocation(const GLchar* name) const;
	GLuint createShaderProgram() const;
	void CheckShaderCompilation(GLuint shader, ShaderErrorType type) const;

private:
	GLuint m_ShaderID;
	std::string m_VertexShaderCode;
	std::string m_FragmentShaderCode;

};

#endif // !_SHADER_H_