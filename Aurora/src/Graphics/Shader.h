#pragma once

#include "Core/Base.h"
#include "ShaderResource.h"

#include <string>
#include <map>
#include <unordered_map>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

/*
 * The std::vector<uint32_t>s used are basically the byte buffers for the binaries, however spriv and shaderc dont use single 
 * uint8_t bytes rather they use "Words" of 4 bytes each.
 */

namespace Aurora {

	// To be expanded...
	enum class ShaderType : uint8_t
	{
		None = 0,
		TwoStageVertFrag,
		Compute
	};

	enum class ShaderStageType : uint8_t
	{
		None = 0,
		Vertex,
		Fragment
	};

	enum class ShaderUniformType : uint8_t
	{
		None = 0,
		Bool,
		Int,
		UInt,
		Float,
		IVec2,
		IVec3,
		IVec4,
		Vec2, 
		Vec3, 
		Vec4,
		Mat3,
		Mat4
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

	struct ShaderProperties
	{
		std::string Name;
		std::filesystem::path AssetPath;
		ShaderType Type = ShaderType::TwoStageVertFrag;
	};

	class Shader : public RefCountedObject
	{
	private:
		using ShaderStage = uint32_t; /*GLenum*/

	public:
		// Push Constant buffer that contains uniforms
		struct ShaderPushBuffer
		{
			std::string Name;
			uint32_t Size = 0;
			std::unordered_map<std::string, ShaderUniform> Uniforms;
		};

	public:
		Shader() = default;
		Shader(const ShaderProperties& props);
		Shader(const std::string& filePath, ShaderType type);
		virtual ~Shader();

		void Bind() const;
		void UnBind() const;

		static Ref<Shader> Create(const std::string& filepath, ShaderType type = ShaderType::TwoStageVertFrag);
		static Ref<Shader> Create(const ShaderProperties& props);

		bool Reload();
		// TODO: Maybe move into Renderer since it should be the one responsible for dispatching compute shaders to do work!
		void Dispatch(uint32_t dimX, uint32_t dimY, uint32_t dimZ) const;

		size_t GetHash() const;
		// Returns last time modified of the asset path in minutes
		uint32_t GetLastTimeModified() const;

		// Setting uniforms...
		void SetUniform(const std::string& fullname, float value) const;
		void SetUniform(const std::string& fullname, int value) const;
		void SetUniform(const std::string& fullname, uint32_t value) const;
		void SetUniform(const std::string& fullname, const glm::ivec2& value) const;
		void SetUniform(const std::string& fullname, const glm::ivec3& value) const;
		void SetUniform(const std::string& fullname, const glm::ivec4& value) const;
		void SetUniform(const std::string& fullname, const glm::vec2& value) const;
		void SetUniform(const std::string& fullname, const glm::vec3& value) const;
		void SetUniform(const std::string& fullname, const glm::vec4& value) const;
		void SetUniform(const std::string& fullname, const glm::mat3& value) const;
		void SetUniform(const std::string& fullname, const glm::mat4& value) const;

		inline bool IsCompute() const { return m_ShaderType == ShaderType::Compute; }

		inline const std::string& GetName() const { return m_Name; }
		inline const std::filesystem::path& GetFilePath() const { return m_AssetPath; }
		const ShaderResourceDeclaration* GetShaderResource(const std::string& name) const;
		inline bool CompilationFailed() const { return m_CompilationFailed; }
		std::string GetTypeString() const;
		inline static constexpr uint32_t GetCompileTimeThreshold() { return s_CompileTimeThreshold; }
		
		inline const std::unordered_map<std::string, ShaderPushBuffer>& GetShaderBuffers() const { return m_Buffers; }
		inline const std::unordered_map<std::string, ShaderResourceDeclaration>& GetShaderResources() const { return m_Resources; }

		static std::vector<Ref<Shader>> AllShaders;

	private:
		void Load2Stage(const std::string& source);
		void LoadCompute(const std::string& source);
		void CreateProgram();

		bool CompileOrGetVulkanBinary(const std::unordered_map<ShaderStage, std::string>& shaderSources, bool forceCompile);
		bool CompileOrGetOpenGLBinary(bool forceCompile);
		void Reflect(ShaderStage type, const std::vector<uint32_t>& shaderData);

		std::string PreprocessComputeSource(const std::string& source);
		std::unordered_map<uint32_t/*GLenum*/, std::string> SplitSource(const std::string& source);

		// Currently not used...
		void UploadUniformInt(uint32_t location, int32_t value) const;
		void UploadUniformIntArray(uint32_t location, int32_t* values, int32_t count) const;
		void UploadUniformFloat(uint32_t location, float value) const;
		void UploadUniformFloat2(uint32_t location, const glm::vec2& value) const;
		void UploadUniformFloat3(uint32_t location, const glm::vec3& value) const;
		void UploadUniformFloat4(uint32_t location, const glm::vec4& value) const;
		void UploadUniformMat3(uint32_t location, const glm::mat3& value) const;
		void UploadUniformMat4(uint32_t location, const glm::mat4& value) const;
		void UploadUniformMat4Array(uint32_t location, const glm::mat4& values, uint32_t count) const;
		void UploadUniformInt(const std::string& name, int32_t value) const;
		void UploadUniformUInt(const std::string& name, uint32_t value) const;
		void UploadUniformIntArray(const std::string& name, int32_t* values, uint32_t count) const;
		void UploadUniformFloat(const std::string& name, float value) const;
		void UploadUniformFloat2(const std::string& name, const glm::vec2& value) const;
		void UploadUniformFloat3(const std::string& name, const glm::vec3& value) const;
		void UploadUniformFloat4(const std::string& name, const glm::vec4& value) const;
		void UploadUniformMat4(const std::string& name, const glm::mat4& value) const;

		int GetUniformLocation(const std::string& name) const;

	private:
		uint32_t m_ShaderID = 0;

		std::string m_Name;
		std::filesystem::path m_AssetPath;
		ShaderType m_ShaderType = ShaderType::TwoStageVertFrag;
		bool m_CompilationFailed = false;

		std::unordered_map<ShaderStage, std::string> m_OpenGLShaderSource; // OpenGL Source Code...
		std::unordered_map<ShaderStage, std::vector<uint32_t>> m_VulkanSPIRV;
		std::unordered_map<ShaderStage, std::vector<uint32_t>> m_OpenGLSPIRV;

		std::unordered_map<std::string, ShaderPushBuffer> m_Buffers;
		std::unordered_map<std::string, ShaderResourceDeclaration> m_Resources;
		mutable std::unordered_map<std::string, int> m_UniformLocations;

		// If the shader's last time modified is less than 5 minutes, recompilation occurs
		constexpr static uint32_t s_CompileTimeThreshold = 5;

	};


	// This is owned by the renderer and allows for easy shader managing throughout the engine
	class ShaderLibrary
	{
	public:
		ShaderLibrary() = default;
		~ShaderLibrary() = default;

		static Scope<ShaderLibrary> Create();

		// Adds an already created shader into the library
		void Add(const Ref<Shader>& shader);

		// Loads a shader into the library
		Ref<Shader> Load(const std::string& filepath, ShaderType type = ShaderType::TwoStageVertFrag);
		Ref<Shader> Load(const ShaderProperties& props); // Gives the option to specify name

		Ref<Shader> TryGet(const std::string& name) const;
		const Ref<Shader>& Get(const std::string& name) const;

		bool Exist(const std::string& name) const;
		bool ExistHash(const size_t hash) const;

	private:
		mutable std::map<size_t, Ref<Shader>> m_ShadersWithHash;

	};

}