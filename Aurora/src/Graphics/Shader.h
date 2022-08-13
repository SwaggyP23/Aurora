#pragma once

#include "Core/Base.h"

#include <string>
#include <unordered_map>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace Aurora {

	namespace Utils {

		enum class ShaderErrorType : uint16_t
		{
			None = 0, VertexShader, FragmentShader, GeometryShader
		};

	}

	//enum class ShaderUniformType : uint8_t
	//{
	//	None = 0,
	//	Bool,
	//	Int, UInt,
	//	Float,
	//	Vec2, Vec3, Vec4,
	//	Mat3, Mat4,
	//	IVec2, IVec3, IVec4
	//};

	//// A class for uniform information
	//class ShaderUniform
	//{
	//public:
	//	ShaderUniform() = default;
	//	ShaderUniform(const std::string& name, ShaderUniformType type, uint32_t size, uint32_t offset);

	//	const std::string& GetName() const { return m_Name; }
	//	ShaderUniformType GetUniformType() const { return m_Type; }
	//	uint32_t GetSize() const { return m_Size; }
	//	uint32_t GetOffset() const { return m_Offset; }

	//	static const std::string& UniformTypeToString(ShaderUniformType type);

	//private:
	//	std::string m_Name;
	//	ShaderUniformType m_Type = ShaderUniformType::None;
	//	uint32_t m_Size = 0;
	//	uint32_t m_Offset = 0;

	//};

	//// A uniform buffer has an amount of uniforms
	//struct ShaderUniformBuffer
	//{
	//	std::string Name;
	//	uint32_t Index;
	//	uint32_t BindingPoint;
	//	uint32_t Size;
	//	uint32_t BufferID;
	//	std::vector<ShaderUniform> Uniforms;
	//};

	//struct ShaderBuffer
	//{
	//	std::string Name;
	//	uint32_t Size = 0;
	//	std::unordered_map<std::string, ShaderUniform> Uniforms;
	//};

	class Shader : public RefCountedObject
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

		// This is temporary untill i have an asset manager
		static std::vector<Ref<Shader>> s_Shaders;

	private:
		std::unordered_map<uint32_t/*GLenum*/, std::string> SplitSource(const std::string& source);
		uint32_t CreateShaderProgram(const std::unordered_map<uint32_t/*GLenum*/, std::string>& shaderSources) const;
		int GetUniformLocation(const std::string& name) const;
		void CheckProgramLinkage(uint32_t program, const std::vector<uint32_t>& shaderIDs) const;
		void CheckShaderCompilation(uint32_t shader, Utils::ShaderErrorType type) const;

	private:
		uint32_t m_ShaderID = 0;
		std::string m_FilePath;
		std::string m_Name;

		mutable std::unordered_map<std::string, int> m_UniformLocations;

	};


	// This is owned by the renderer and allows for easy shader managing throughout the solution
	class ShaderLibrary
	{
	public:
		ShaderLibrary() = default;
		~ShaderLibrary() = default;

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