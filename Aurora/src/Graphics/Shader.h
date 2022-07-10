#pragma once

#include "Aurora.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace Aurora {

	class Shader
	{
	public:
		Shader(const std::string& filePath);
		~Shader();

		void bind() const;
		void unBind() const;

		static Ref<Shader> Create(const std::string& filepath);

		// Setting uniforms...

		void setUniform1i(const GLchar* name, GLuint val) const;
		void setUniformArrayi(const GLchar* name, GLint* vals, uint32_t count) const;
		void setUniform1f(const GLchar* name, GLfloat val) const;
		void setUniform2f(const GLchar* name, const glm::vec2& vector) const;
		void setUniform3f(const GLchar* name, const glm::vec3& vector) const;
		void setUniform4f(const GLchar* name, const glm::vec4& vector) const;
		void setUniformMat3(const GLchar* name, const glm::mat3& matrix) const;
		void setUniformMat3(const GLchar* name, const float* matrix) const;
		void setUniformMat4(const GLchar* name, const glm::mat4& matrix) const;
		void setUniformMat4(const GLchar* name, const float* matrix) const;

		inline const std::string& getName() const { return m_Name; }

	private:
		std::unordered_map<GLenum, std::string> splitSource(const std::string& source);
		GLuint createShaderProgram(const std::unordered_map<GLenum, std::string>& shaderSources) const;
		GLint getUniformLocation(const std::string& name) const;

	private:
		GLuint m_ShaderID;
		std::string m_FilePath;
		std::string m_Name;

		mutable std::unordered_map<std::string, GLint> m_UniformLocations;

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
		std::unordered_map<std::string, Ref<Shader>> m_Shaders{};

	};

}