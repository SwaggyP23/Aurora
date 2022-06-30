#pragma once

#ifndef _SHADER_H_
#define _SHADER_H_

#include "OGLpch.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

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
	void setUniform2f(const GLchar* name, const glm::vec2& vector) const;
	void setUniform3f(const GLchar* name, const glm::vec3& vector) const;
	void setUniform4f(const GLchar* name, const glm::vec4& vector) const;
	void setUniformMat3(const GLchar* name, const glm::mat3& matrix) const;
	void setUniformMat3(const GLchar* name, const float* matrix) const;
	void setUniformMat4(const GLchar* name, const glm::mat4& matrix) const;
	void setUniformMat4(const GLchar* name, const float* matrix) const;

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