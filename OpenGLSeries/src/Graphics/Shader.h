#pragma once

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
	std::unordered_map<GLenum, std::string> splitSource(const std::string& source); 
	GLuint createShaderProgram(const std::unordered_map<GLenum, std::string>& shaderSources) const;
	GLint getUniformLocation(const GLchar* name) const;

private:
	GLuint m_ShaderID;
	std::string m_FilePath;

	//std::unordered_map<GLenum, std::string> m_ShaderSources; // Storing the shaderType with its source
	// However this is currently useless since we have the shader asset files present

};


// This is owned by the renderer and allows for easy shader managing throughout the solution
class ShaderLibrary
{
public:
	void Add(const Ref<Shader>& shader);
	void Add(const std::string& name, const Ref<Shader>& shader);

	Ref<Shader> Load(const std::string& filepath);
	Ref<Shader> Load(const std::string& name, const std::string& filepath); // Gives the option to specify name

	Ref<Shader> Get(const std::string& name);

	bool Exists(const std::string& name) const;

private:
	std::unordered_map<std::string, Ref<Shader>> m_Shaders;

};