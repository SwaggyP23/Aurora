#pragma once

#include "Core/Base.h"
#include "ShaderResource.h"

#include <string>
#include <unordered_map>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

/*
 * The std::vector<uint32_t>s used are basically the byte buffers for the binaries, however spriv and shaderc dont use single 
 * uint8_t bytes rather they use "Words" of 4 bytes each.
 * Currently i will not be using push_constants, rather i will be heavily using uniform buffers which is what i was adviced to
 * do by @implodee(chat link is found in my public server [SwaggyP's Server]), so yea use UBOs for all the info.
 * 
 * NOTE: If you in your shader create a push_constant block and for some reason one of its members is not used throughout the
 * shader, or if it is used but then overridin by something else, for example:
 * layout(push_constant) uniform Mats
 * {
 *		float a;
 *		float b;
 * };
 * 
 * ... 
 * 
 * o_Color = vec4(a);
 * o_Color = vec4(b);
 * 
 * then the member "a" of the push_constant block will not be converted into an opengl uniform and it wont be found when calling
 * glGetUniformLocation(shaderID, name);
 * Therefore be careful of what members you are putting in the push_constant blocks and if you are using them or not!
 * 
 * TODO: Add better reloading workflow. Since now if i change something little in the vertex shader, the reload actually reloads
 * the vertex and also the fragment shader, which is inefficient so need to find out a way to see what changed and then recompile
 * that specific stage...!
 */

// Define as 1 if you want the shader to use std::fstream.
#define AR_NO_USE_FILE_POINTER 0

namespace Aurora {

	enum class ShaderStageType : uint8_t
	{
		None = 0, Vertex, Fragment, Geometry
	};

	enum class ShaderType : uint8_t
	{
		None = 0,
		TwoStageVertFrag,
		Compute
	};

	enum class ShaderUniformType : uint8_t
	{
		None = 0,
		Bool,
		Int, UInt,
		Float,
		IVec2, IVec3, IVec4,
		Vec2, Vec3, Vec4,
		Mat3, Mat4
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
		std::string AssetPath;
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
		Shader(const ShaderProperties& props, bool forceCompile);
		Shader(const std::string& filePath, bool forceCompile);
		~Shader();

		void Bind() const;
		void UnBind() const;

		static Ref<Shader> Create(const std::string& filepath, bool forceCompile = false);
		static Ref<Shader> Create(const ShaderProperties& props, bool forceCompile = false);

		void Reload(bool forceCompile = true);

		[[nodiscard]] size_t GetHash() const;

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

		[[nodiscard]] inline bool IsCompute() const { return m_ShaderType == ShaderType::Compute; }

		[[nodiscard]] inline const std::string& GetName() const { return m_Name; }
		[[nodiscard]] inline const std::string& GetFilePath() const { return m_AssetPath; }
		[[nodiscard]] const ShaderResourceDeclaration* GetShaderResource(const std::string& name) const;
		
		[[nodiscard]] inline const std::unordered_map<std::string, ShaderPushBuffer>& GetShaderBuffers() const { return m_Buffers; }
		[[nodiscard]] inline const std::unordered_map<std::string, ShaderResourceDeclaration>& GetShaderResources() const { return m_Resources; }

		// This is temporary untill i have an asset manager. It kind of acts as an asset manager lol having all the shaders
		static std::vector<Ref<Shader>> s_AllShaders;

	private:
		void Load2Stage(const std::string& source, bool forceCompile = false);
		void LoadCompute(const std::string& source, bool forceCompile = false);
		void CreateProgram();

		void CompileOrGetVulkanBinary(const std::unordered_map<ShaderStage, std::string>& shaderSources, bool forceCompile);
		void CompileOrGetOpenGLBinary(bool forceCompile);
		void Reflect(ShaderStage type, const std::vector<uint32_t>& shaderData);

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
		std::string m_AssetPath;
		ShaderType m_ShaderType = ShaderType::TwoStageVertFrag;

		std::unordered_map<ShaderStage, std::string> m_OpenGLShaderSource; // OpenGL Source Code...
		std::unordered_map<ShaderStage, std::vector<uint32_t>> m_VulkanSPIRV;
		std::unordered_map<ShaderStage, std::vector<uint32_t>> m_OpenGLSPIRV;

		std::unordered_map<std::string, ShaderPushBuffer> m_Buffers;
		std::unordered_map<std::string, ShaderResourceDeclaration> m_Resources;
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