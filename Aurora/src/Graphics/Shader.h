#pragma once

#include "Core/Base.h"

#include <string>
#include <unordered_map>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace Aurora {

	class Shader
	{
	public:
		Shader(const std::string& filePath);
		~Shader();

		void Bind() const;
		void UnBind() const;

		static Ref<Shader> Create(const std::string& filepath);

		// Setting uniforms...

		void SetUniform1i(const char* name, uint32_t val) const;
		void SetUniformArrayi(const char* name, int* vals, uint32_t count) const;
		void SetUniform1f(const char* name, float val) const;
		void SetUniform2f(const char* name, const glm::vec2& vector) const;
		void SetUniform3f(const char* name, const glm::vec3& vector) const;
		void SetUniform4f(const char* name, const glm::vec4& vector) const;
		void SetUniformMat3(const char* name, const glm::mat3& matrix) const;
		void SetUniformMat3(const char* name, const float* matrix) const;
		void SetUniformMat4(const char* name, const glm::mat4& matrix) const;
		void SetUniformMat4(const char* name, const float* matrix) const;

		inline const std::string& GetName() const { return m_Name; }
		inline const std::string& GetFilePath() const { return m_FilePath; }

	private:
		std::unordered_map<uint32_t/*GLenum*/, std::string> SplitSource(const std::string& source);
		uint32_t CreateShaderProgram(const std::unordered_map<uint32_t/*GLenum*/, std::string>& shaderSources) const;
		int GetUniformLocation(const std::string& name) const;

	private:
		uint32_t m_ShaderID;
		std::string m_FilePath;
		std::string m_Name;

		mutable std::unordered_map<std::string, int> m_UniformLocations;

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