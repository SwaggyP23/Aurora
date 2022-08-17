#pragma once

#include "Core/Base.h"

#include <string>
#include <unordered_map>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

/*
 * The std::vector<uint32_t>s used are basically the byte buffers for the binaries, however spriv and shaderc dont use single 
 * uint8_t bytes rather they use "Words" of 4 bytes each
 */

namespace spirv_cross {

	class CompilerGLSL;

}

namespace Aurora {

	enum class ShaderErrorType : uint8_t
	{
		None = 0, Vertex, Fragment, Compute, Geometry
	};

	enum class ShaderUniformType : uint8_t
	{
		None = 0,
		Bool,
		Int, UInt,
		Float,
		Vec2, Vec3, Vec4,
		Mat3, Mat4,
		IVec2, IVec3, IVec4
	};

	// A representation of a uniform
	class ShaderUniform
	{
	public:
		ShaderUniform() = default;
		ShaderUniform(const std::string& name, ShaderUniformType type, uint32_t size, uint32_t offset);

		const std::string& GetName() const { return m_Name; }
		ShaderUniformType GetUniformType() const { return m_Type; }
		uint32_t GetSize() const { return m_Size; }
		uint32_t GetOffset() const { return m_Offset; }

		static std::string UniformTypeToString(ShaderUniformType type);

	private:
		std::string m_Name;
		ShaderUniformType m_Type = ShaderUniformType::None;
		uint32_t m_Size = 0;
		uint32_t m_Offset = 0;

	};

	// Shader side buffer that contains uniforms
	struct ShaderBuffer
	{
		std::string Name;
		uint32_t Size = 0;
		std::unordered_map<std::string, ShaderUniform> Uniforms;
	};

	class Shader : public RefCountedObject
	{
	public:
		Shader() = default;
		Shader(const std::string& filePath, bool forceCompile);
		~Shader();

		void Bind() const;
		void UnBind() const;

		static Ref<Shader> Create(const std::string& filepath, bool forceCompile = false);

		size_t GetHash() const;

		void Reload(bool forceCompile = false) {} // TODO: Implement with dynamic shader reloading...

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
		inline const std::string& GetFilePath() const { return m_AssetPath; }

		// This is temporary untill i have an asset manager
		static std::vector<Ref<Shader>> s_Shaders;

	private:
		void Load(const std::string& source, bool forceCompile);
		void CreateProgram();

		void CompileOrGetVulkanBinary(const std::unordered_map<uint32_t/*GLenum*/, std::string>& shaderSources, bool forceCompile = false);
		void CompileOrGetOpenGLBinary(bool forceCompile = false);
		void Reflect(uint32_t/*GLenum*/type, const std::vector<uint32_t>& shaderData);

		void ParseConstantBuffer(const spirv_cross::CompilerGLSL& compiler);

		std::unordered_map<uint32_t/*GLenum*/, std::string> SplitSource(const std::string& source);
		int GetUniformLocation(const std::string& name) const;

	private:
		uint32_t m_ShaderID = 0;

		std::string m_Name;
		std::string m_AssetPath;

		uint32_t m_ConstantBufferOffset = 0;

		bool IsLoaded = false;
		bool m_IsCompute = false;

		// String sources
		std::unordered_map<uint32_t/*GLenum*/, std::string> m_ShaderSource; // OpenGL Source Code...
		std::unordered_map<uint32_t/*GLenum*/, std::vector<uint32_t>> m_VulkanSPIRV;
		std::unordered_map<uint32_t/*GLenum*/, std::vector<uint32_t>> m_OpenGLSPIRV;

		std::unordered_map<std::string, ShaderBuffer> m_Buffers;
		mutable std::unordered_map<std::string, int> m_UniformLocations;

	};


	// This is owned by the renderer and allows for easy shader managing throughout the engine
	class ShaderLibrary
	{
	public:
		ShaderLibrary() = default;
		~ShaderLibrary() = default;

		// Adds an already created shader into the library
		void Add(const Ref<Shader>& shader);
		void Add(const std::string& name, const Ref<Shader>& shader);

		// Loads a shader into the library
		void Load(const std::string& filepath, bool forceCompile = false);
		void Load(const std::string& name, const std::string& filepath); // Gives the option to specify name

		const Ref<Shader>& Get(const std::string& name);

		bool Exist(const std::string& name) const;

	private:
		std::unordered_map<std::string, Ref<Shader>> m_Shaders{};

	};

}