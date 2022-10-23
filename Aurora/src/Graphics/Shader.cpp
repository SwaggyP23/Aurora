#include "Aurorapch.h"
#include "Shader.h"

#include "Utils/UtilFunctions.h"

#include <glad/glad.h>

#include <shaderc/shaderc.hpp>
#include <spirv_cross/spirv_glsl.hpp>
#include <spirv-tools/libspirv.hpp>

// Define as 1 if you want the shader to use std::fstream.
#define AR_NO_USE_FILE_POINTER 0

// FOR DEBUGGING!!!!
// Define if you want the shader dissassembly after compilation to print to console
//! #define AR_PRINT_SHADER_DISSASSEMBLY

namespace Aurora {

	static uint32_t s_SharedPushConstantAttributeOffset = 0;

	std::vector<Ref<Shader>> Shader::AllShaders;

	namespace Utils {

		constexpr inline static const char* GetCacheDirectory(bool vulkanApi)
		{
			if(vulkanApi)
				return "Resources/cache/shaders/Vulkan";
			else
				return "Resources/cache/shaders/OpenGL";
		}

		inline static void CreateCacheDirIfNeeded(bool vulkanApi)
		{
			std::filesystem::path cacheDir = GetCacheDirectory(vulkanApi);
			if (!std::filesystem::exists(cacheDir))
				std::filesystem::create_directories(cacheDir);
		}

		inline static uint32_t/*GLenum*/ ShaderTypeFromString(std::string_view type)
		{
			if (type == "vertex")
				return GL_VERTEX_SHADER;
			if (type == "fragment")
				return GL_FRAGMENT_SHADER;
			if (type == "compute")
				return GL_COMPUTE_SHADER;

			AR_CORE_ASSERT(false, "Unknown shader type!");
			return 0;
		}

		inline static ShaderStageType ErrorTypeFromShaderType(uint32_t/*GLenum*/ shaderStageType)
		{
			switch (shaderStageType)
			{
			    case GL_VERTEX_SHADER:		return ShaderStageType::Vertex;
			    case GL_FRAGMENT_SHADER:	return ShaderStageType::Fragment;
			}

			AR_CORE_ASSERT(false, "Unknown shader type!");
			return ShaderStageType::None;
		}

		inline static std::string ShaderTypeString(ShaderType type)
		{
			switch (type)
			{
			    case ShaderType::TwoStageVertFrag:   return "Two Stage Program";
			    case ShaderType::Compute:			 return "Compute";
			}

			AR_CORE_ASSERT(false, "Unknown shader type!");
			return "";
		}

		inline static const char* GLShaderTypeCachedVulkanFileExtension(uint32_t/*GLenum*/ type)
		{
			switch (type)
			{
			    case GL_VERTEX_SHADER:        return ".cachedVulkan.vert";
			    case GL_FRAGMENT_SHADER:      return ".cachedVulkan.frag";
			    case GL_COMPUTE_SHADER:       return ".cachedVulkan.comp";
			}

			AR_CORE_ASSERT(false, "Unknown Shader Type");
			return "U Messed Up";
		}

		inline static const char* GLShaderTypeCachedOpenGLFileExtension(uint32_t/*GLenum*/ type)
		{
			switch (type)
			{
			    case GL_VERTEX_SHADER:        return ".cachedOpenGL.vert";
			    case GL_FRAGMENT_SHADER:      return ".cachedOpenGL.frag";
			    case GL_COMPUTE_SHADER:       return ".cachedOpenGL.comp";
			}

			AR_CORE_ASSERT(false, "Unknown Shader Type");
			return "U Messed Up";
		}

		inline static shaderc_shader_kind GLShaderTypeToShaderC(uint32_t/*GLenum*/ type)
		{
			switch (type)
			{
			    case GL_VERTEX_SHADER:            return shaderc_vertex_shader;
			    case GL_FRAGMENT_SHADER:          return shaderc_fragment_shader;
			    case GL_COMPUTE_SHADER:           return shaderc_compute_shader;
			}

			AR_CORE_ASSERT(false, "Unknown Shader Type!");
			return (shaderc_shader_kind)0;
		}

		inline static const char* GLShaderTypeToString(uint32_t/*GLenum*/ type)
		{
			switch (type)
			{
			    case GL_VERTEX_SHADER:            return "Vertex";
			    case GL_FRAGMENT_SHADER:          return "Fragment";
			    case GL_COMPUTE_SHADER:           return "Compute";
			}

			AR_CORE_ASSERT(false, "Unknown Shader Type");
			return "";
		}

		inline static ShaderUniformType SPIRTypeToShaderUniformType(spirv_cross::SPIRType type)
		{
			switch (type.basetype)
			{
			    case spirv_cross::SPIRType::Boolean:  return ShaderUniformType::Bool;
			    case spirv_cross::SPIRType::Int:
				{
			    	if (type.vecsize == 1)            return ShaderUniformType::Int;
			    	if (type.vecsize == 2)            return ShaderUniformType::IVec2;
			    	if (type.vecsize == 3)            return ShaderUniformType::IVec3;
			    	if (type.vecsize == 4)            return ShaderUniformType::IVec4;
				}
			    case spirv_cross::SPIRType::UInt:     return ShaderUniformType::UInt;
			    case spirv_cross::SPIRType::Float:
				{
					// Test for Mat types since if we test for vectors first we can return the wrong type since the default value of vecsize = 1
			    	if (type.columns == 3)            return ShaderUniformType::Mat3;
			    	if (type.columns == 4)            return ShaderUniformType::Mat4;

			    	if (type.vecsize == 1)            return ShaderUniformType::Float;
			    	if (type.vecsize == 2)            return ShaderUniformType::Vec2;
			    	if (type.vecsize == 3)            return ShaderUniformType::Vec3;
			    	if (type.vecsize == 4)            return ShaderUniformType::Vec4;
				}
			}

			AR_CORE_ASSERT(false, "Unknown type!");
			return ShaderUniformType::None;
		}

	}

	////////////////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////////////
    ///////////// ShaderLibrary
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////////////////////////////////

	Scope<ShaderLibrary> ShaderLibrary::Create()
	{
		return CreateScope<ShaderLibrary>();
	}

	void ShaderLibrary::Add(const Ref<Shader>& shader)
	{
		// Hash the name of the shader which supposibely should be UNIQUE
		const size_t hash = std::hash<std::string>{}(shader->GetName());
		AR_CORE_ASSERT(!ExistHash(hash), "Shader already exists!");

		m_ShadersWithHash[hash] = shader;
	}

	Ref<Shader> ShaderLibrary::Load(const ShaderProperties& props)
	{
		Ref<Shader> result = Shader::Create(props);
		Add(result);

		return result;
	}

	Ref<Shader> ShaderLibrary::Load(const std::string& filepath, ShaderType type)
	{
		Ref<Shader> result = Shader::Create(filepath, type);
		Add(result);

		return result;
	}

	Ref<Shader> ShaderLibrary::TryGet(const std::string& name) const
	{
		const size_t hash = std::hash<std::string>{}(name);
		if (ExistHash(hash))
			return m_ShadersWithHash[hash];

		AR_CORE_ERROR_TAG("ShaderLibrary", "Coutld not find shader {0}", name.c_str());
		return nullptr;
	}

	const Ref<Shader>& ShaderLibrary::Get(const std::string& name) const
	{
		const size_t hash = std::hash<std::string>{}(name);
		AR_CORE_ASSERT(ExistHash(hash), "Shader not found!");
		return m_ShadersWithHash[hash];
	}

	bool ShaderLibrary::Exist(const std::string& name) const
	{
		const size_t hash = std::hash<std::string>{}(name);
		return ExistHash(hash);
	}

	bool ShaderLibrary::ExistHash(const size_t hash) const
	{
		return m_ShadersWithHash.find(hash) != m_ShadersWithHash.end();
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////////////////////////////////
	///////////// Shader Uniform
	////////////////////////////////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////////////////////////////////

	ShaderUniform::ShaderUniform(const std::string& name, ShaderUniformType type, uint32_t size, uint32_t offset)
		: m_Name(name), m_Type(type), m_Size(size), m_Offset(offset)
	{
	}

	std::string ShaderUniform::UniformTypeToString(ShaderUniformType type)
	{
		switch (type)
		{
		    case ShaderUniformType::Bool:       return "Bool";
		    case ShaderUniformType::Int:	    return "Int";
		    case ShaderUniformType::UInt:	    return "UInt";
			case ShaderUniformType::Float:      return "Float";
			case ShaderUniformType::IVec2:		return "IVec2";
			case ShaderUniformType::IVec3:      return "IVec3";
		    case ShaderUniformType::IVec4:		return "IVec4";
		    case ShaderUniformType::Vec2:		return "Vec2";
		    case ShaderUniformType::Vec3:		return "Vec3";
		    case ShaderUniformType::Vec4:		return "Vec4";
			case ShaderUniformType::Mat3:		return "Mat3";
			case ShaderUniformType::Mat4:		return "Mat4";
		}

		AR_CORE_ASSERT(false, "Unknown Shader Uniform Type!");
		return "";
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////////////////////////////////
	///////////// Shader
	////////////////////////////////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////////////////////////////////

	Ref<Shader> Shader::Create(const std::string& filepath, ShaderType type)
	{
		Ref<Shader> result = nullptr;

		result = CreateRef<Shader>(filepath, type);

		if(std::find(AllShaders.begin(), AllShaders.end(), result) == AllShaders.end())
			AllShaders.push_back(result);

		return result;
	}

	Ref<Shader> Shader::Create(const ShaderProperties& props)
	{
		Ref<Shader> result = nullptr;

		result = CreateRef<Shader>(props);

		if (std::find(AllShaders.begin(), AllShaders.end(), result) == AllShaders.end())
			AllShaders.push_back(result);

		return result;
	}

	Shader::Shader(const ShaderProperties& props)
		: m_Name(props.Name), m_AssetPath(props.AssetPath), m_ShaderType(props.Type)
	{
		AR_PROFILE_FUNCTION();

		switch (props.Type)
		{
			case ShaderType::TwoStageVertFrag:
			{
				std::string shaderFullSource = Utils::FileIO::ReadTextFile(props.AssetPath);
				Load2Stage(shaderFullSource);
				break;
			}
			case ShaderType::Compute:
			{
				std::string shaderFullSource = Utils::FileIO::ReadTextFile(props.AssetPath);
				LoadCompute(shaderFullSource);
				break;
			}
		}
	}

	Shader::Shader(const std::string& filePath, ShaderType type)
		: m_AssetPath(filePath), m_ShaderType(type)
	{
		AR_PROFILE_FUNCTION();

		// Extracting the name from filepath...
		{
			size_t lastSlash = filePath.find_last_of("/\\");
			lastSlash = lastSlash == std::string::npos ? 0 : lastSlash + 1;

			size_t lastDot = filePath.rfind('.');
			size_t count = lastDot == std::string::npos ? filePath.size() - lastSlash : lastDot - lastSlash;
			m_Name = filePath.substr(lastSlash, count);
		}

		// Creating program...
		{
			switch (type)
			{
				case ShaderType::TwoStageVertFrag:
				{
					std::string shaderFullSource = Utils::FileIO::ReadTextFile(filePath);
					Load2Stage(shaderFullSource);
					break;
				}
				case ShaderType::Compute:
				{
					std::string shaderFullSource = Utils::FileIO::ReadTextFile(filePath);
					LoadCompute(shaderFullSource);
					break;
				}
			}
		}
	}

	Shader::~Shader()
	{
		glDeleteProgram(m_ShaderID);
	}

	// TODO: Make it so that when shader reloading fails the engine does not crash!
	bool Shader::Reload()
	{
		Timer timer;
		bool failedCompilation = false;

		// Copied incase shader compilation fails... we can revert to the old shader
		std::unordered_map<ShaderStage, std::vector<uint32_t>> vulkanSPIR_VCopy = m_VulkanSPIRV;
		std::unordered_map<ShaderStage, std::vector<uint32_t>> openGLSPIR_VCopy = m_OpenGLSPIRV;

		// Clean the old program object
		if (m_ShaderID)
		{
			glDeleteProgram(m_ShaderID);
			m_ShaderID = 0;

			m_OpenGLShaderSource.clear();
			m_OpenGLSPIRV.clear();
			m_VulkanSPIRV.clear();
			m_Buffers.clear();
			m_Resources.clear();
			m_UniformLocations.clear();
		}

		std::string shaderFullSource = Utils::FileIO::ReadTextFile(m_AssetPath);

		switch (m_ShaderType)
		{
			case ShaderType::TwoStageVertFrag:		m_OpenGLShaderSource = SplitSource(shaderFullSource); break;
			case ShaderType::Compute:				m_OpenGLShaderSource[GL_COMPUTE_SHADER] = PreprocessComputeSource(shaderFullSource); break;
			default:
				AR_CORE_ASSERT(false, "Unknown shader type!");
		}
		//m_OpenGLShaderSource = SplitSource(shaderFullSource);

		constexpr bool forceCompile = true;

		if (CompileOrGetVulkanBinary(m_OpenGLShaderSource, forceCompile) == false || CompileOrGetOpenGLBinary(forceCompile) == false)
		{
			// NOTE: 
			// Revert to old SPIR-V binaries and reflect and reload the program on those...
			// Another approach could be that if the compilation fails we could revert and not call CreateProgram and thus not changine the loaded
			// program however it is better to just reload the whole program and reflect on it again just in case the we want to see reflection data
			m_VulkanSPIRV = vulkanSPIR_VCopy;
			m_OpenGLSPIRV = openGLSPIR_VCopy;

			failedCompilation = true;
		}

		CreateProgram();
		if (!failedCompilation)
		{
			AR_CONSOLE_LOG_INFO("Reloading shader: {0} took {1} milliseconds", m_Name, timer.ElapsedMillis());
			AR_CORE_WARN_TAG("Renderer", "Reloading shader: {0} took {1} milliseconds", m_Name, timer.ElapsedMillis());
		}

		m_CompilationFailed = failedCompilation;
		return failedCompilation;
	}

	void Shader::Dispatch(uint32_t dimX, uint32_t dimY, uint32_t dimZ) const
	{
		AR_CORE_ASSERT(m_ShaderType == ShaderType::Compute, "Shader has to be a compute shader in order to dispatch it!");
		glDispatchCompute(dimX, dimY, dimZ);
	}

	void Shader::Load2Stage(const std::string& source)
	{
		// At this stage it contains split Vulkan source code
		m_OpenGLShaderSource = SplitSource(source);

		bool forceCompile = GetLastTimeModified() < 5 ? true : false;

		{
			Timer timer;
			CompileOrGetVulkanBinary(m_OpenGLShaderSource, forceCompile);
			CompileOrGetOpenGLBinary(forceCompile);
			CreateProgram();
			AR_CORE_WARN_TAG("Renderer", "Shader creation took {0}ms", timer.ElapsedMillis());
		}
	}

	void Shader::LoadCompute(const std::string& source)
	{
		m_OpenGLShaderSource[GL_COMPUTE_SHADER] = PreprocessComputeSource(source);

		bool forceCompile = GetLastTimeModified() < 5 ? true : false;

		{
			Timer timer;
			CompileOrGetVulkanBinary(m_OpenGLShaderSource, forceCompile);
			CompileOrGetOpenGLBinary(forceCompile);
			CreateProgram();
			AR_CORE_WARN_TAG("Renderer", "Shader creation took {0}ms", timer.ElapsedMillis());
		}
	}

	bool Shader::CompileOrGetVulkanBinary(const std::unordered_map<ShaderStage, std::string>& shaderSources, bool forceCompile)
	{
		AR_PROFILE_FUNCTION();

		Utils::CreateCacheDirIfNeeded(true);
		std::filesystem::path cacheDir = Utils::GetCacheDirectory(true);

		for (const auto& [type, source] : shaderSources)
		{
			std::filesystem::path cachedPath = cacheDir / (m_AssetPath.filename().string() + Utils::GLShaderTypeCachedVulkanFileExtension(type));
			std::string p = cachedPath.string();

#if AR_NO_USE_FILE_POINTER
			std::fstream f1(p.c_str(), std::ios::in | std::ios::binary);
			if (f1.is_open())
			{
				f1.seekg(0, std::ios::end);
				uint64_t size = f1.tellg();
				f1.seekg(0, std::ios::beg);
				m_VulkanSPIRV[type].resize(size / sizeof(uint32_t));
				f1.read((char*)m_VulkanSPIRV[type].data(), size);
				f1.close();
			}
#else
			FILE* f1;
			fopen_s(&f1, p.c_str(), "rb"); // read binary
			if (f1 && !forceCompile)
			{
				fseek(f1, 0, SEEK_END);
				uint64_t size = ftell(f1);
				fseek(f1, 0, SEEK_SET);
				m_VulkanSPIRV[type].resize(size / sizeof(uint32_t));
				fread(m_VulkanSPIRV[type].data(), sizeof(uint32_t), size / sizeof(uint32_t), f1);
				fclose(f1);
			}
#endif
			else
			{
				// Need to close the opened file if forceCompile is set to true so that it does not leak
#if AR_NO_USE_FILE_POINTER
				if (f1.is_open())
					f1.close();
#else
				if (f1)
					fclose(f1);
#endif

				shaderc::Compiler compiler;
				shaderc::CompileOptions options;

				options.SetTargetEnvironment(shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_3);
				options.AddMacroDefinition("OPENGL");
#ifdef AR_PRINT_SHADER_DISSASSEMBLY
				options.SetGenerateDebugInfo(); // This provides the source when using SPIRV_TOOLS and dissassembling
#endif
				options.SetAutoMapLocations(true);
				//options.SetAutoSampledTextures(true);

				// Not optimizing shaders when in Vulkan format since that would break stuff once in OGL format!
				constexpr bool optimize = false;
				if (optimize)
					options.SetOptimizationLevel(shaderc_optimization_level_performance);

				shaderc::SpvCompilationResult result = compiler.CompileGlslToSpv(source, Utils::GLShaderTypeToShaderC(type), m_AssetPath.string().c_str(), options);
				if (result.GetCompilationStatus() != shaderc_compilation_status_success)
				{
					AR_CONSOLE_LOG_ERROR("Compilation Error in Stage: {0}", Utils::GLShaderTypeToString(type));
					AR_CONSOLE_LOG_ERROR("{0}", result.GetErrorMessage());
					
					return false;
				}

				m_VulkanSPIRV[type] = std::vector<uint32_t>(result.cbegin(), result.cend());

#if AR_NO_USE_FILE_POINTER
				std::fstream f2(p.c_str(), std::ios::out | std::ios::binary);
				if (f2.is_open())
				{
					f2.write((char*)m_VulkanSPIRV[type].data(), m_VulkanSPIRV[type].size() * sizeof(uint32_t));
					f2.close();
				}
#else
				FILE* f2;
				fopen_s(&f2, p.c_str(), "wb"); // write binary
				if (f2)
				{
					fwrite(m_VulkanSPIRV[type].data(), sizeof(uint32_t), m_VulkanSPIRV[type].size(), f2);
					fclose(f2);
				}
#endif
				else
				{
					AR_CORE_ERROR_TAG("Renderer", "Could not open file for writing '{0}'", p);
				}
			}
		}

		return true;
	}

	bool Shader::CompileOrGetOpenGLBinary(bool forceCompile)
	{
		AR_PROFILE_FUNCTION();

		Utils::CreateCacheDirIfNeeded(false);
		std::filesystem::path cacheDir = Utils::GetCacheDirectory(false);

	 	uint16_t PushBinding = 0;
		for (const auto& [type, spirv] : m_VulkanSPIRV)
		{
			std::filesystem::path cachedPath = cacheDir / (m_AssetPath.filename().string() + Utils::GLShaderTypeCachedOpenGLFileExtension(type));
			std::string p = cachedPath.string();

			spirv_cross::CompilerGLSL glslCompiler = spirv_cross::CompilerGLSL(spirv);
			const auto& pushConstResources = glslCompiler.get_shader_resources().push_constant_buffers;
			for (int i = 0; i < pushConstResources.size(); i++)
			{
				glslCompiler.set_decoration(pushConstResources[i].id, spv::DecorationLocation, PushBinding++);
			}

			// At this stage it contains split OpenGL source code
			m_OpenGLShaderSource[type] = glslCompiler.compile();

#if AR_NO_USE_FILE_POINTER
			std::fstream f1(p.c_str(), std::ios::in | std::ios::binary);
			if (f1.is_open())
			{
				f1.seekg(0, std::ios::end);
				uint64_t size = f1.tellg();
				f1.seekg(0, std::ios::beg);
				m_OpenGLSPIRV[type].resize(size / sizeof(uint32_t));
				f1.read((char*)m_OpenGLSPIRV[type].data(), size);
				f1.close();
			}
#else
			FILE* f1;
			fopen_s(&f1, p.c_str(), "rb"); // read binary
			if (f1 && !forceCompile)
			{
				fseek(f1, 0, SEEK_END);
				uint64_t size = ftell(f1);
				fseek(f1, 0, SEEK_SET);
				m_OpenGLSPIRV[type].resize(size / sizeof(uint32_t));
				fread(m_OpenGLSPIRV[type].data(), sizeof(uint32_t), size / sizeof(uint32_t), f1);
				fclose(f1);
			}
#endif
			else
			{
				// Need to close the opened file if forceCompile is set to true so that it does not leak
#if AR_NO_USE_FILE_POINTER
				if (f1.is_open())
					f1.close();
#else
				if (f1)
					fclose(f1);
#endif

				shaderc::Compiler compiler;
				shaderc::CompileOptions options;

				options.SetTargetEnvironment(shaderc_target_env_opengl, shaderc_env_version_opengl_4_5);
				options.SetGenerateDebugInfo(); // This provides the source when using SPIRV_TOOLS and dissassembling
				//options.SetAutoSampledTextures(true);

				// Optimize shaders once in OpenGL format!
				constexpr bool optimize = true;
				if (optimize)
					options.SetOptimizationLevel(shaderc_optimization_level_performance);

				// Technically this is useless since the openGL source being compiled is trans generated by shaderc which supporibely should be correct
				// but we check anyways just in case!
				shaderc::SpvCompilationResult result = compiler.CompileGlslToSpv(m_OpenGLShaderSource[type], Utils::GLShaderTypeToShaderC(type), m_AssetPath.string().c_str(), options);
				if (result.GetCompilationStatus() != shaderc_compilation_status_success)
				{
					AR_CONSOLE_LOG_ERROR("Compilation Error in Stage: {0}", Utils::GLShaderTypeToString(type));
					AR_CONSOLE_LOG_ERROR("{0}", result.GetErrorMessage());

					return false;
				}

				m_OpenGLSPIRV[type] = std::vector<uint32_t>(result.cbegin(), result.cend());
#ifdef AR_PRINT_SHADER_DISSASSEMBLY
				spvtools::SpirvTools tools(SPV_ENV_OPENGL_4_5);
				std::string debugSPVReturn;
				tools.Disassemble(m_OpenGLSPIRV[type], &debugSPVReturn);
				AR_WARN("Shader: {0} - {1}\n{2}", m_Name, Utils::GLShaderTypeToString(type), debugSPVReturn);
#endif

#if AR_NO_USE_FILE_POINTER
				std::fstream f2(p.c_str(), std::ios::out | std::ios::binary);
				if (f2.is_open())
				{
					f2.write((char*)m_OpenGLSPIRV[type].data(), m_OpenGLSPIRV[type].size() * sizeof(uint32_t));
					f2.close();
				}
#else
				FILE* f2;
				fopen_s(&f2, p.c_str(), "wb"); // write binary
				if (f2)
				{
					fwrite(m_OpenGLSPIRV[type].data(), sizeof(uint32_t), m_OpenGLSPIRV[type].size(), f2);
					fclose(f2);
				}
#endif
				else
				{
					AR_CORE_ERROR_TAG("Renderer", "Could not open file for writing '{0}'", p);
					AR_CORE_ASSERT(false);
				}
			}
		}

		return true;
	}

	void Shader::CreateProgram()
	{
		GLuint program = glCreateProgram();

		std::vector<GLuint> shaderIDs;
		for (const auto& [type, spirv] : m_OpenGLSPIRV)
		{
			GLuint shaderID = glCreateShader(type);
			shaderIDs.push_back(shaderID);

			glShaderBinary(1, &shaderID, GL_SHADER_BINARY_FORMAT_SPIR_V, spirv.data(), (GLsizei)spirv.size() * sizeof(uint32_t));
			glSpecializeShader(shaderID, "main", 0, nullptr, nullptr);
			glAttachShader(program, shaderID);
		}

		glLinkProgram(program);

		GLint link;
		glGetProgramiv(program, GL_LINK_STATUS, &link);
		if (!link)
		{
			GLint length = 0;
			glGetProgramiv(program, GL_INFO_LOG_LENGTH, &length);

			char* message = (char*)alloca(length * sizeof(char));
			glGetProgramInfoLog(program, length, &length, message);
			AR_CORE_ERROR("[Program Linkage] Shader linking failed! {0}:\n\t{1}", m_AssetPath.string(), std::string(message));

			glDeleteProgram(program);

			for (uint32_t id : shaderIDs)
				glDeleteShader(id);
		}

		for (uint32_t id : shaderIDs)
		{
			glDetachShader(program, id);
			glDeleteShader(id);
		}

		m_ShaderID = program;

		// Reflection happens after the shaders have been created otherwise we cant know stuff about the sampled images
		s_SharedPushConstantAttributeOffset = 0;
		for (const auto& [type, data] : m_VulkanSPIRV)
			Reflect(type, data);

		glUseProgram(m_ShaderID);
		// If no push_constant blocks are found, this loop will not enter and thus no uniforms are there to query their location
		for (const auto& [bufferName, buffer] : m_Buffers)
		{
			for (const auto& [name, uniform] : buffer.Uniforms)
			{
				GLint location = glGetUniformLocation(m_ShaderID, name.c_str());
				if (location == -1)
					AR_CORE_WARN_TAG("Renderer", "Could not find uniform location {0}", name);

				m_UniformLocations[name] = location;
			}
		}
		glUseProgram(0);
	}

	void Shader::Reflect(ShaderStage type, const std::vector<uint32_t>& shaderData)
	{
		AR_PROFILE_FUNCTION();

		spirv_cross::CompilerGLSL compiler(shaderData);
		spirv_cross::ShaderResources resources = compiler.get_shader_resources();

		AR_CORE_TRACE("[REFLECT] ==============================");
		AR_CORE_TRACE("[REFLECT] {0} - {1}", Utils::GLShaderTypeToString(type), m_AssetPath.string());
		AR_CORE_TRACE("[REFLECT] \t{0} Uniform Buffers", resources.uniform_buffers.size());
		AR_CORE_TRACE("[REFLECT] \t{0} Push Constant Buffers", resources.push_constant_buffers.size());
		AR_CORE_TRACE("[REFLECT] \t{0} Storage Buffers", resources.storage_buffers.size());
		AR_CORE_TRACE("[REFLECT] \t{0} Sampled Images", resources.sampled_images.size());
		AR_CORE_TRACE("[REFLECT] \t{0} Storage Images", resources.storage_images.size());


		AR_CORE_TRACE("[REFLECT] ------------------------------");

		AR_CORE_TRACE("[REFLECT] Uniform Buffers:");
		for (const spirv_cross::Resource& res : resources.uniform_buffers)
		{
			const spirv_cross::SPIRType& bufferType = compiler.get_type(res.base_type_id);
			uint32_t bufferSize = (uint32_t)compiler.get_declared_struct_size(bufferType);
			uint32_t binding = compiler.get_decoration(res.id, spv::DecorationBinding);
			int memberCount = (int)bufferType.member_types.size();

			AR_CORE_TRACE("[REFLECT] \tName: {0}", res.name);
			AR_CORE_TRACE("[REFLECT] \t   Size: {0}", bufferSize);
			AR_CORE_TRACE("[REFLECT] \t   Binding: {0}", binding);
			AR_CORE_TRACE("[REFLECT] \t   Member Count: {0}", memberCount);
		}
		AR_CORE_TRACE("[REFLECT] ------------------------------");

		AR_CORE_TRACE("[REFLECT] Storage Buffers:");
		for (const spirv_cross::Resource& res : resources.storage_buffers)
		{
			const std::string& bufferName = res.name;
			const spirv_cross::SPIRType& bufferType = compiler.get_type(res.base_type_id);
			uint32_t bufferSize = (uint32_t)compiler.get_declared_struct_size(bufferType);
			uint32_t binding = compiler.get_decoration(res.id, spv::DecorationBinding);
			uint32_t memberCount = (uint32_t)bufferType.member_types.size();

			AR_CORE_TRACE("[REFLECT] \tName: {0}", bufferName);
			AR_CORE_TRACE("[REFLECT] \t   Size: {0}", bufferSize);
			AR_CORE_TRACE("[REFLECT] \t   Binding: {0}", binding);
			AR_CORE_TRACE("[REFLECT] \t   Member Count: {0}", memberCount);
		}
		AR_CORE_TRACE("[REFLECT] ------------------------------");

		AR_CORE_TRACE("[REFLECT] Push Constant Buffers:");
		for (const spirv_cross::Resource& res : resources.push_constant_buffers)
		{
			const std::string& bufferName = res.name;
			const spirv_cross::SPIRType& bufferType = compiler.get_type(res.base_type_id);
			uint32_t bufferSize = (uint32_t)compiler.get_declared_struct_size(bufferType);
			uint32_t memberCount = (uint32_t)bufferType.member_types.size();
			uint32_t location = compiler.get_decoration(res.id, spv::DecorationLocation);

			AR_CORE_TRACE("[REFLECT] \tName: {0}", bufferName);
			AR_CORE_TRACE("[REFLECT] \t   Size: {0}", bufferSize);
			AR_CORE_TRACE("[REFLECT] \t   Location: {0}", location);
			AR_CORE_TRACE("[REFLECT] \t   Member Count: {0}", memberCount);

			// We create and insert a ShaderPushBuffer into the map to later on be used in the material to get the uniform location
			ShaderPushBuffer& buffer = m_Buffers[bufferName];
			buffer.Name = bufferName;
			buffer.Size = bufferSize;

			for (uint32_t i = 0; i < memberCount; i++)
			{
				const spirv_cross::SPIRType& type = compiler.get_type(bufferType.member_types[i]);
				const std::string& memberName = compiler.get_member_name(bufferType.self, i);
				uint32_t memberSize = (uint32_t)compiler.get_declared_struct_member_size(bufferType, i);
				uint32_t memberOffset = compiler.type_struct_member_offset(bufferType, i) + s_SharedPushConstantAttributeOffset;

				std::string UniformName = fmt::format("{}.{}", bufferName, memberName);
				buffer.Uniforms[UniformName] = ShaderUniform{ UniformName, Utils::SPIRTypeToShaderUniformType(type), memberSize, memberOffset };
			}
			s_SharedPushConstantAttributeOffset += bufferSize;
		}
		AR_CORE_TRACE("[REFLECT] ------------------------------");

		AR_CORE_TRACE("[REFLECT] Sampled Images:");
		int32_t sampler = 0;
		for (const spirv_cross::Resource& res : resources.sampled_images)
		{
			const std::string& name = res.name;
			const spirv_cross::SPIRType& type = compiler.get_type(res.base_type_id);
			uint32_t binding = compiler.get_decoration(res.id, spv::DecorationBinding);
			uint32_t dimension = type.image.dim;

			GLint location = glGetUniformLocation(m_ShaderID, name.c_str());
			AR_CORE_ASSERT(location != -1);
			m_Resources[name] = ShaderResourceDeclaration(name, binding, 1);
			// This is redundant since the the material sets it and shader sets the binding itself for the textures
			// glProgramUniform1i(m_ShaderID, location, binding);

			AR_CORE_TRACE("[REFLECT] \tName: {0}", name);
			AR_CORE_TRACE("[REFLECT] \t   Binding: {0}", binding);
		}
		AR_CORE_TRACE("[REFLECT] ------------------------------");

		AR_CORE_TRACE("[REFLECT] Storage Images:");
		for (const spirv_cross::Resource& res : resources.storage_images)
		{
			const std::string& name = res.name;
			const spirv_cross::SPIRType& type = compiler.get_type(res.base_type_id);
			uint32_t binding = compiler.get_decoration(res.id, spv::DecorationBinding);
			uint32_t dimension = type.image.dim;

			GLint location = glGetUniformLocation(m_ShaderID, name.c_str());
			AR_CORE_ASSERT(location != -1);
			m_Resources[name] = ShaderResourceDeclaration(name, binding, 1);
			// This is redundant since the the material sets it and shader sets the binding itself for the textures 
			// glProgramUniform1i(m_ShaderID, location, binding);

			AR_CORE_TRACE("[REFLECT] \tName: {0}", name);
			AR_CORE_TRACE("[REFLECT] \t    Binding: {0}", binding);
		}
		AR_CORE_TRACE("[REFLECT] ------------------------------");
	}

	std::string Shader::PreprocessComputeSource(const std::string& source)
	{
		constexpr const char* typeIdentifier = "#pragma";
		uint32_t typeIdentifierLength = (uint32_t)strlen(typeIdentifier);
		size_t pos = source.find(typeIdentifier, 0);

		uint32_t eol = (uint32_t)source.find_first_of("\r\n", pos); // End of shader type declaration line
		AR_CORE_ASSERT(eol != std::string::npos, "Syntax error!");

		uint32_t typeBegin = (uint32_t)pos + typeIdentifierLength + 1; // Getting the type
		std::string type = source.substr(typeBegin, eol - typeBegin);
		AR_CORE_ASSERT(type == "compute", "Type of shader should be compute since this function ran!");

		uint32_t nextLinePos = (uint32_t)source.find_first_not_of("\r\n", eol); // Start of shader code
		return source.substr(nextLinePos);
	}

	// This should only be for Load2Stage and no compute shaders should pass through this
	std::unordered_map<uint32_t/*GLenum*/, std::string> Shader::SplitSource(const std::string& source)
	{ // Props to @TheCherno
		std::unordered_map<uint32_t/*GLenum*/, std::string> shaderSources;

		constexpr const char* typeIdentifier = "#pragma";
		uint32_t typeIdentifierLength = (uint32_t)strlen(typeIdentifier);
		size_t pos = source.find(typeIdentifier, 0);

		while (pos != std::string::npos)
		{
			size_t eol = source.find_first_of("\r\n", pos); // End of shader type declaration line
			AR_CORE_ASSERT(eol != std::string::npos, "Syntax error!");

			size_t typeBegin = pos + typeIdentifierLength + 1; // Getting the type
			std::string type = source.substr(typeBegin, eol - typeBegin);
			AR_CORE_ASSERT(type == "vertex" || type == "fragment", "Unknown Shader Type!");

			size_t nextLinePos = source.find_first_not_of("\r\n", eol); // Start of shader code

			pos = source.find(typeIdentifier, nextLinePos); // Start of next shader type declaration line

			uint32_t shaderType = Utils::ShaderTypeFromString(type);
			shaderSources[shaderType] = (pos == std::string::npos) ? source.substr(nextLinePos) : source.substr(nextLinePos, pos - nextLinePos);
		}

		return shaderSources;
	}

	void Shader::Bind() const
	{
		glUseProgram(m_ShaderID);
	}

	void Shader::UnBind() const
	{
		glUseProgram(0);
	}

	// TODO: Needs rework and it should hash the shader source not the asset path
	size_t Shader::GetHash() const
	{
		return std::hash<std::filesystem::path>{}(m_AssetPath);
	}

	std::string Shader::GetTypeString() const
	{
		return Utils::ShaderTypeString(m_ShaderType);
	}

	uint32_t Shader::GetLastTimeModified() const
	{
		// Reference: https://stackoverflow.com/questions/61030383/how-to-convert-stdfilesystemfile-time-type-to-time-t
		std::filesystem::file_time_type lastTime = std::filesystem::last_write_time(m_AssetPath);
		uint64_t ticks = lastTime.time_since_epoch().count() - std::filesystem::__std_fs_file_time_epoch_adjustment;

		// create a time_point from ticks
		Timer::SystemClock::time_point tp{ Timer::SystemClock::time_point::duration(ticks) };

		return std::chrono::duration_cast<Timer::Minutes>(Timer::SystemClock::now() - tp).count();
	}

	const ShaderResourceDeclaration* Shader::GetShaderResource(const std::string& name) const
	{
		if (m_Resources.find(name) == m_Resources.end())
			return nullptr;

		return &(m_Resources.at(name));
	}

	int Shader::GetUniformLocation(const std::string& name) const
	{
		int location = glGetUniformLocation(m_ShaderID, name.c_str());
		if (location == -1)
			AR_CORE_WARN_TAG("Renderer", "Could not find uniform '{0}' in shader: {1}", name, m_Name);

		return location;
	}

	void Shader::SetUniform(const std::string& fullname, float value) const
	{
		AR_CORE_ASSERT(m_UniformLocations.find(fullname) != m_UniformLocations.end());
		GLint location = m_UniformLocations.at(fullname);
		glProgramUniform1f(m_ShaderID, location, value);
	}

	void Shader::SetUniform(const std::string& fullname, int value) const
	{
		AR_CORE_ASSERT(m_UniformLocations.find(fullname) != m_UniformLocations.end());
		GLint location = m_UniformLocations.at(fullname);
		glProgramUniform1i(m_ShaderID, location, value);
	}

	void Shader::SetUniform(const std::string& fullname, uint32_t value) const
	{
		AR_CORE_ASSERT(m_UniformLocations.find(fullname) != m_UniformLocations.end());
		GLint location = m_UniformLocations.at(fullname);
		glProgramUniform1ui(m_ShaderID, location, value);
	}

	void Shader::SetUniform(const std::string& fullname, const glm::ivec2& value) const
	{
		AR_CORE_ASSERT(m_UniformLocations.find(fullname) != m_UniformLocations.end());
		GLint location = m_UniformLocations.at(fullname);
		glProgramUniform2i(m_ShaderID, location, value.x, value.y);
	}

	void Shader::SetUniform(const std::string& fullname, const glm::ivec3& value) const
	{
		AR_CORE_ASSERT(m_UniformLocations.find(fullname) != m_UniformLocations.end());
		GLint location = m_UniformLocations.at(fullname);
		glProgramUniform3i(m_ShaderID, location, value.x, value.y, value.z);
	}

	void Shader::SetUniform(const std::string& fullname, const glm::ivec4& value) const
	{
		AR_CORE_ASSERT(m_UniformLocations.find(fullname) != m_UniformLocations.end());
		GLint location = m_UniformLocations.at(fullname);
		glProgramUniform4i(m_ShaderID, location, value.x, value.y, value.z, value.w);
	}

	void Shader::SetUniform(const std::string& fullname, const glm::vec2& value) const
	{
		AR_CORE_ASSERT(m_UniformLocations.find(fullname) != m_UniformLocations.end());
		GLint location = m_UniformLocations.at(fullname);
		glProgramUniform2fv(m_ShaderID, location, 1, glm::value_ptr(value));
	}

	void Shader::SetUniform(const std::string& fullname, const glm::vec3& value) const
	{
		AR_CORE_ASSERT(m_UniformLocations.find(fullname) != m_UniformLocations.end());
		GLint location = m_UniformLocations.at(fullname);
		glProgramUniform3fv(m_ShaderID, location, 1, glm::value_ptr(value));
	}

	void Shader::SetUniform(const std::string& fullname, const glm::vec4& value) const
	{
		AR_CORE_ASSERT(m_UniformLocations.find(fullname) != m_UniformLocations.end());
		GLint location = m_UniformLocations.at(fullname);
		glProgramUniform4fv(m_ShaderID, location, 1, glm::value_ptr(value));
	}

	void Shader::SetUniform(const std::string& fullname, const glm::mat3& value) const
	{
		AR_CORE_ASSERT(m_UniformLocations.find(fullname) != m_UniformLocations.end());
		GLint location = m_UniformLocations.at(fullname);
		glProgramUniformMatrix3fv(m_ShaderID, location, 1, GL_FALSE, glm::value_ptr(value));
	}

	void Shader::SetUniform(const std::string& fullname, const glm::mat4& value) const
	{
		AR_CORE_ASSERT(m_UniformLocations.find(fullname) != m_UniformLocations.end());
		GLint location = m_UniformLocations.at(fullname);
		glProgramUniformMatrix4fv(m_ShaderID, location, 1, GL_FALSE, glm::value_ptr(value));
	}

#pragma region Currently not in use!

	void Shader::UploadUniformInt(uint32_t location, int32_t value) const
	{
		glProgramUniform1i(m_ShaderID, location, value);
	}

	void Shader::UploadUniformIntArray(uint32_t location, int32_t* values, int32_t count) const
	{
		glProgramUniform1iv(m_ShaderID, location, count, values);
	}

	void Shader::UploadUniformFloat(uint32_t location, float value) const
	{
		glProgramUniform1f(m_ShaderID, location, value);
	}

	void Shader::UploadUniformFloat2(uint32_t location, const glm::vec2& value) const
	{
		glProgramUniform2f(m_ShaderID, location, value.x, value.y);
	}

	void Shader::UploadUniformFloat3(uint32_t location, const glm::vec3& value) const
	{
		glProgramUniform3f(m_ShaderID, location, value.x, value.y, value.z);
	}

	void Shader::UploadUniformFloat4(uint32_t location, const glm::vec4& value) const
	{
		glProgramUniform4f(m_ShaderID, location, value.x, value.y, value.z, value.w);
	}

	void Shader::UploadUniformMat3(uint32_t location, const glm::mat3& value) const
	{
		glProgramUniformMatrix3fv(m_ShaderID, location, 1, GL_FALSE, glm::value_ptr(value));
	}

	void Shader::UploadUniformMat4(uint32_t location, const glm::mat4& value) const
	{
		glProgramUniformMatrix4fv(m_ShaderID, location, 1, GL_FALSE, glm::value_ptr(value));
	}

	void Shader::UploadUniformMat4Array(uint32_t location, const glm::mat4& values, uint32_t count) const
	{
		glProgramUniformMatrix4fv(m_ShaderID, location, count, GL_FALSE, glm::value_ptr(values));
	}

	void Shader::UploadUniformInt(const std::string& name, int32_t value) const
	{
		int32_t location = GetUniformLocation(name);
		glProgramUniform1i(m_ShaderID, location, value);
	}

	void Shader::UploadUniformUInt(const std::string& name, uint32_t value) const
	{
		int32_t location = GetUniformLocation(name);
		glProgramUniform1ui(m_ShaderID, location, value);
	}

	void Shader::UploadUniformIntArray(const std::string& name, int32_t* values, uint32_t count) const
	{
		int32_t location = GetUniformLocation(name);
		glProgramUniform1iv(m_ShaderID, location, count, values);
	}

	void Shader::UploadUniformFloat(const std::string& name, float value) const
	{
		int32_t location = GetUniformLocation(name);
		glProgramUniform1f(m_ShaderID, location, value);
	}

	void Shader::UploadUniformFloat2(const std::string& name, const glm::vec2& value) const
	{
		int32_t location = GetUniformLocation(name);
		glProgramUniform2f(m_ShaderID, location, value.x, value.y);
	}

	void Shader::UploadUniformFloat3(const std::string& name, const glm::vec3& value) const
	{
		int32_t location = GetUniformLocation(name);
		glProgramUniform3f(m_ShaderID, location, value.x, value.y, value.z);
	}

	void Shader::UploadUniformFloat4(const std::string& name, const glm::vec4& value) const
	{
		int32_t location = GetUniformLocation(name);
		glProgramUniform4f(m_ShaderID, location, value.x, value.y, value.z, value.w);
	}

	void Shader::UploadUniformMat4(const std::string& name, const glm::mat4& value) const
	{
		int32_t location = GetUniformLocation(name);
		glProgramUniformMatrix4fv(m_ShaderID, location, 1, GL_FALSE, glm::value_ptr(value));
	}

#pragma endregion

}