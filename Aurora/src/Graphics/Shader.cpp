#include "Aurorapch.h"
#include "Shader.h"

#include "Utils/UtilFunctions.h"

#include <glad/glad.h>

#include <shaderc/shaderc.hpp>
#include <spirv_cross/spirv_glsl.hpp>
#include <spirv-tools/libspirv.hpp>

namespace Aurora {

	std::vector<Ref<Shader>> Shader::s_AllShaders;

	namespace Utils {

		static const char* GetCacheDirectory()
		{
			return "Resources/cache/shaders/OpenGL";
		}

		static void CreateCacheDirIfNeeded()
		{
			std::string cacheDir = GetCacheDirectory();
			if (!std::filesystem::exists(cacheDir))
				std::filesystem::create_directories(cacheDir);
		}

		static uint32_t/*GLenum*/ ShaderTypeFromString(const std::string& type)
		{
			if (type == "vertex")
				return GL_VERTEX_SHADER;
			else if (type == "fragment")
				return GL_FRAGMENT_SHADER;
			else if (type == "compute")
				return GL_COMPUTE_SHADER;
			else if (type == "geometry")
				return GL_GEOMETRY_SHADER;

			AR_CORE_ASSERT(false, "Unknown shader type!");
			return 0;
		}

		static ShaderErrorType ErrorTypeFromShaderType(uint32_t/*GLenum*/ shaderType)
		{
			switch (shaderType)
			{
			    case GL_VERTEX_SHADER:		return ShaderErrorType::Vertex;
			    case GL_FRAGMENT_SHADER:	return ShaderErrorType::Fragment;
			    case GL_COMPUTE_SHADER:	    return ShaderErrorType::Compute;
			    case GL_GEOMETRY_SHADER:	return ShaderErrorType::Geometry;
			}

			AR_CORE_ASSERT(false, "Unknown shader type!");
			return ShaderErrorType::None;
		}

		static const char* GLShaderTypeCachedVulkanFileExtension(uint32_t type)
		{
			switch (type)
			{
			    case GL_VERTEX_SHADER:        return ".cachedVulkan.vert";
			    case GL_FRAGMENT_SHADER:      return ".cachedVulkan.frag";
			    case GL_GEOMETRY_SHADER:      return ".cachedVulkan.geo";
			    case GL_COMPUTE_SHADER:       return ".cachedVulkan.comp";
			}

			AR_CORE_ASSERT(false, "Unknown Shader Type");
			return "";
		}

		static const char* GLShaderTypeCachedOpenGLFileExtension(uint32_t type)
		{
			switch (type)
			{
			    case GL_VERTEX_SHADER:        return ".cachedOpenGL.vert";
			    case GL_FRAGMENT_SHADER:      return ".cachedOpenGL.frag";
			    case GL_GEOMETRY_SHADER:      return ".cachedOpenGL.geo";
			    case GL_COMPUTE_SHADER:       return ".cachedOpenGL.comp";
			}

			AR_CORE_ASSERT(false, "Unknown Shader Type");
			return "";
		}

		static shaderc_shader_kind GLShaderTypeToShaderC(uint32_t type)
		{
			switch (type)
			{
			    case GL_VERTEX_SHADER:            return shaderc_vertex_shader;
			    case GL_FRAGMENT_SHADER:          return shaderc_fragment_shader;
			    case GL_GEOMETRY_SHADER:          return shaderc_geometry_shader;
			    case GL_COMPUTE_SHADER:           return shaderc_compute_shader;
			}

			AR_CORE_ASSERT(false, "Unknown Shader Type!");
			return (shaderc_shader_kind)0;
		}

		static const char* GLShaderTypeToString(uint32_t type)
		{
			switch (type)
			{
			    case GL_VERTEX_SHADER:            return "Vertex";
			    case GL_FRAGMENT_SHADER:          return "Fragment";
			    case GL_GEOMETRY_SHADER:          return "Geometry";
			    case GL_COMPUTE_SHADER:           return "Compute";
			}

			AR_CORE_ASSERT(false, "Unknown Shader Type");
			return "";
		}

		static ShaderUniformType SPIRTypeToShaderUniformType(spirv_cross::SPIRType type)
		{
			switch (type.basetype)
			{
			    case spirv_cross::SPIRType::Boolean:  return ShaderUniformType::Bool;
			    case spirv_cross::SPIRType::Int:
			    	if (type.vecsize == 1)            return ShaderUniformType::Int;
			    	if (type.vecsize == 2)            return ShaderUniformType::IVec2;
			    	if (type.vecsize == 3)            return ShaderUniformType::IVec3;
			    	if (type.vecsize == 4)            return ShaderUniformType::IVec4;
			    case spirv_cross::SPIRType::UInt:     return ShaderUniformType::UInt;
			    case spirv_cross::SPIRType::Float:
			    
			    	if (type.columns == 3)            return ShaderUniformType::Mat3;
			    	if (type.columns == 4)            return ShaderUniformType::Mat4;
			    
			    	if (type.vecsize == 1)            return ShaderUniformType::Float;
			    	if (type.vecsize == 2)            return ShaderUniformType::Vec2;
			    	if (type.vecsize == 3)            return ShaderUniformType::Vec3;
			    	if (type.vecsize == 4)            return ShaderUniformType::Vec4;
			    	break;
			}

			AR_CORE_ASSERT(false, "Unknown type!");
			return ShaderUniformType::None;
		}

	}

	////////////////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// ShaderLibrary!!!!!!!!!
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void ShaderLibrary::Add(const std::string& name, const Ref<Shader>& shader)
	{
		AR_PROFILE_FUNCTION();

		AR_CORE_ASSERT(!Exist(name), "Shader already exists!");
		m_Shaders[name] = shader;
	}

	void ShaderLibrary::Add(const Ref<Shader>& shader)
	{
		AR_PROFILE_FUNCTION();

		const std::string& name = shader->GetName();
		AR_CORE_ASSERT(!Exist(name), "Shader already exists!");
		Add(name, shader);
	}

	void ShaderLibrary::Load(const std::string& filepath, bool forceCompile)
	{
		Add(Shader::Create(filepath, forceCompile));
	}

	void ShaderLibrary::Load(const std::string& name, const std::string& filepath)
	{
		Add(name, Shader::Create(filepath));
	}

	const Ref<Shader>& ShaderLibrary::Get(const std::string& name)
	{
		AR_PROFILE_FUNCTION();

		AR_CORE_ASSERT(Exist(name), "Shader not found!");
		return m_Shaders[name];
	}

	bool ShaderLibrary::Exist(const std::string& name) const // TODO: See wassup with this since it always crashes
	{
		return m_Shaders.find(name) != m_Shaders.end();
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// Shader Uniform!!!!!!!!!
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
		    case Aurora::ShaderUniformType::Bool:      return std::string("Bool");
		    case Aurora::ShaderUniformType::Int:       return std::string("Int");
			case Aurora::ShaderUniformType::Float:     return std::string("Float");
		}

		AR_CORE_ASSERT(false, "Unknown Shader Uniform Type!");
		return "None";
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// Shader!!!!!!!!!!!!!!!!!
	////////////////////////////////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////////////////////////////////

	size_t Shader::GetHash() const
	{
		return std::hash<std::string>{}(m_AssetPath);
	}

	Ref<Shader> Shader::Create(const std::string& filepath, bool forceCompile)
	{
		Ref<Shader> result = nullptr;

		result = CreateRef<Shader>(filepath, forceCompile);

		if(std::find(s_AllShaders.begin(), s_AllShaders.end(), result) == s_AllShaders.end())
			s_AllShaders.push_back(result);

		return result;
	}

	Shader::Shader(const std::string& filePath, bool forceCompile)
		: m_AssetPath(filePath)
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
			std::string shaderFullSource = Utils::FileIO::ReadTextFile(filePath);
			Load(shaderFullSource, forceCompile);
		}
	}

	Shader::~Shader()
	{
		AR_PROFILE_FUNCTION();

		glDeleteProgram(m_ShaderID);
	}

	void Shader::Reload(bool forceCompile)
	{
		// Clean the old program object
		if (m_ShaderID)
			glDeleteProgram(m_ShaderID);

		Timer timer;
		std::string shaderFullSource = Utils::FileIO::ReadTextFile(m_AssetPath);
		m_OpenGLShaderSource = SplitSource(shaderFullSource);

		Utils::CreateCacheDirIfNeeded();

		CompileOrGetVulkanBinary(m_OpenGLShaderSource, forceCompile);
		CompileOrGetOpenGLBinary(forceCompile);
		CreateProgram();
		AR_CORE_WARN_TAG("Shader", "Reloading {0} took {1}ms", m_Name, timer.ElapsedMillis());
	}

	void Shader::Load(const std::string& source, bool forceCompile)
	{
		AR_PROFILE_FUNCTION();

		// At this stage it contains split Vulkan source code
		m_OpenGLShaderSource = SplitSource(source);

		Utils::CreateCacheDirIfNeeded();
	
		{
			Timer timer;
			CompileOrGetVulkanBinary(m_OpenGLShaderSource, forceCompile);
			CompileOrGetOpenGLBinary(forceCompile);
			CreateProgram();
			AR_CORE_WARN_TAG("Shader", "Shader creation took {0}ms", timer.ElapsedMillis());
		}
	}

	void Shader::CompileOrGetVulkanBinary(const std::unordered_map<ShaderStage, std::string>& shaderSources, bool forceCompile)
	{
		AR_PROFILE_FUNCTION();

		std::filesystem::path cacheDir = Utils::GetCacheDirectory();

		//m_VulkanSPIRV.clear();

		for (const auto& [type, source] : shaderSources)
		{
			std::filesystem::path shaderFilePath = m_AssetPath;
			std::filesystem::path cachedPath = cacheDir / (shaderFilePath.filename().string() + Utils::GLShaderTypeCachedVulkanFileExtension(type));
			std::string p = cachedPath.string();

			FILE* f1;
			fopen_s(&f1, p.c_str(), "rb"); // read binary
			if (f1 && !forceCompile)
			{
				fseek(f1, 0, SEEK_END);
				uint64_t size = ftell(f1);
				fseek(f1, 0, SEEK_SET);
				m_VulkanSPIRV[type].resize(size / sizeof(uint32_t));
				fread(m_VulkanSPIRV[type].data(), sizeof(uint32_t), size, f1);
				fclose(f1);
			}
			else
			{
				shaderc::Compiler compiler;
				shaderc::CompileOptions options;

				options.SetTargetEnvironment(shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_3);
				options.AddMacroDefinition("OPENGL");
				options.SetGenerateDebugInfo(); // This provides the source when using SPIRV_TOOLS and dissassembling
				options.SetAutoSampledTextures(false); // TODO: Check what this does!

				// Not optimizing shaders when in Vulkan format!
				constexpr bool optimize = false;
				if (optimize)
					options.SetOptimizationLevel(shaderc_optimization_level_performance);

				shaderc::SpvCompilationResult result = compiler.CompileGlslToSpv(source, Utils::GLShaderTypeToShaderC(type), m_AssetPath.c_str(), options);
				if (result.GetCompilationStatus() != shaderc_compilation_status_success)
				{
					AR_CORE_ERROR_TAG("ShaderC Compilation", result.GetErrorMessage());
					AR_CORE_ASSERT(false);
				}

				m_VulkanSPIRV[type] = std::vector<uint32_t>(result.cbegin(), result.cend());

				if (f1)
					fclose(f1);

				FILE* f2;
				fopen_s(&f2, p.c_str(), "wb"); // write binary
				if (f2)
				{
					fwrite(m_VulkanSPIRV[type].data(), sizeof(uint32_t), m_VulkanSPIRV[type].size(), f2);
					fclose(f2);
				}
				else
				{
					AR_CORE_ERROR_TAG("Shader", "Could not open file for writing '{0}'", p);
				}
			}
		}
	}

	void Shader::CompileOrGetOpenGLBinary(bool forceCompile)
	{
		AR_PROFILE_FUNCTION();

		std::filesystem::path cacheDir = Utils::GetCacheDirectory();

		//m_OpenGLSPIRV.clear();
		short int PushBinding = 0;
		for (const auto& [type, spirv] : m_VulkanSPIRV)
		{
			std::filesystem::path shaderFilePath = m_AssetPath;
			std::filesystem::path cachedPath = cacheDir / (shaderFilePath.filename().string() + Utils::GLShaderTypeCachedOpenGLFileExtension(type));
			std::string p = cachedPath.string();

			// I am doing this step here so that i can get the ogl source code everytime and not only when the shaders are compiled
			spirv_cross::CompilerGLSL glslCompiler = spirv_cross::CompilerGLSL(spirv);
			auto& pushConstResources = glslCompiler.get_shader_resources().push_constant_buffers;
			for (int i = 0; i < pushConstResources.size(); i++)
			{
				glslCompiler.set_decoration(pushConstResources[i].id, spv::DecorationLocation, PushBinding++);
			}

			// At this stage it contains split OpenGL source code
			m_OpenGLShaderSource[type] = glslCompiler.compile();

			FILE* f1;
			fopen_s(&f1, p.c_str(), "rb"); // read binary
			if (f1 && !forceCompile)
			{
				fseek(f1, 0, SEEK_END);
				uint64_t size = ftell(f1);
				fseek(f1, 0, SEEK_SET);
				m_OpenGLSPIRV[type].resize(size / sizeof(uint32_t));
				fread(m_OpenGLSPIRV[type].data(), sizeof(uint32_t), size, f1);
				fclose(f1);
			}
			else
			{
				shaderc::Compiler compiler;
				shaderc::CompileOptions options;

				options.SetTargetEnvironment(shaderc_target_env_opengl, shaderc_env_version_opengl_4_5);
				options.SetGenerateDebugInfo(); // This provides the source when using SPIRV_TOOLS and dissassembling
				options.SetAutoSampledTextures(false); // TODO: Check what this does!
				//options.SetAutoMapLocations(true);

				// Optimize shaders once in OpenGL format!
				constexpr bool optimize = true;
				if (optimize)
					options.SetOptimizationLevel(shaderc_optimization_level_performance);

				shaderc::SpvCompilationResult result = compiler.CompileGlslToSpv(m_OpenGLShaderSource[type], Utils::GLShaderTypeToShaderC(type), m_AssetPath.c_str(), options);
				if (result.GetCompilationStatus() != shaderc_compilation_status_success)
				{
					AR_CORE_ERROR_TAG("ShaderC Compilation", result.GetErrorMessage());
					AR_CORE_ASSERT(false);
				}

				// TODO: Dissassembling the spirv opengl binaries seem to give the same code and nothing optimized
				m_OpenGLSPIRV[type] = std::vector<uint32_t>(result.cbegin(), result.cend());
				spvtools::SpirvTools tools(SPV_ENV_OPENGL_4_5);
				std::string wassup;
				tools.Disassemble(m_OpenGLSPIRV[type], &wassup);
				AR_WARN("Shader: {0} - {1}\n{2}", m_Name, Utils::GLShaderTypeToString(type), wassup);

				if (f1)
					fclose(f1);

				FILE* f2;
				fopen_s(&f2, p.c_str(), "wb"); // write binary
				if (f2)
				{
					fwrite(m_OpenGLSPIRV[type].data(), sizeof(uint32_t), m_OpenGLSPIRV[type].size(), f2);
					fclose(f2);
				}
				else
				{
					AR_CORE_ERROR_TAG("Shader", "Could not open file for writing '{0}'", p);
				}
			}
		}
	}

	void Shader::CreateProgram()
	{
		AR_PROFILE_FUNCTION();

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
			AR_CORE_ERROR_TAG("Program Linkage", "Shader linking failed! {0}:\n\t{1}", m_AssetPath, std::string(message));

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
		for (const auto& [type, data] : m_VulkanSPIRV)
			Reflect(type, data);

		// If no push_constant blocks are found, this loop will not enter and thus no uniforms are there to query their location
		for (auto& [bufferName, buffer] : m_Buffers)
		{
			glUseProgram(m_ShaderID);
			for (auto& [name, uniform] : buffer.Uniforms)
			{
				// glGetUniformLocation return a GL_INVALID_VALUE and the object is not a valid object generated by
				// OpenGL (m_ShaderID) which obviously should be a valid object i cant understand why it wouldnt be
				GLint location = glGetUniformLocation(m_ShaderID, name.c_str());
				if (location == -1)
					AR_CORE_WARN_TAG("Shader", "Could not find uniform location {0}", name);

				m_UniformLocations[name] = location;
			}
			glUseProgram(0);
		}
	}

	void Shader::Reflect(ShaderStage type, const std::vector<uint32_t>& shaderData)
	{
		AR_PROFILE_FUNCTION();

		spirv_cross::CompilerGLSL compiler(shaderData);
		spirv_cross::ShaderResources resources = compiler.get_shader_resources();

		AR_CORE_TRACE_TAG("REFLECT", "==============================");
		AR_CORE_TRACE_TAG("REFLECT", "{0} - {1}", Utils::GLShaderTypeToString(type), m_AssetPath);
		AR_CORE_TRACE_TAG("REFLECT", "\t{0} Uniform Buffers", resources.uniform_buffers.size());
		AR_CORE_TRACE_TAG("REFLECT", "\t{0} Push Constants", resources.push_constant_buffers.size());
		AR_CORE_TRACE_TAG("REFLECT", "\t{0} Resources", resources.sampled_images.size());

		AR_CORE_TRACE_TAG("REFLECT", "------------------------------");

		AR_CORE_TRACE_TAG("REFLECT", "Uniform Buffers:");
		for (const spirv_cross::Resource& res : resources.uniform_buffers)
		{
			const spirv_cross::SPIRType& bufferType = compiler.get_type(res.base_type_id);
			uint32_t bufferSize = (uint32_t)compiler.get_declared_struct_size(bufferType);
			uint32_t binding = compiler.get_decoration(res.id, spv::DecorationBinding);
			int memberCount = (int)bufferType.member_types.size();

			AR_CORE_TRACE_TAG("REFLECT", "\tName: {0}", res.name);
			AR_CORE_TRACE_TAG("REFLECT", "\t   Size: {0}", bufferSize);
			AR_CORE_TRACE_TAG("REFLECT", "\t   Binding: {0}", binding);
			AR_CORE_TRACE_TAG("REFLECT", "\t   Member Count: {0}", memberCount);
			AR_CORE_TRACE_TAG("REFLECT", "------------------------------");
		}

		AR_CORE_TRACE_TAG("REFLECT", "Storage Buffers:");
		for (const spirv_cross::Resource& res : resources.storage_buffers)
		{
			const std::string& bufferName = res.name;
			const spirv_cross::SPIRType& bufferType = compiler.get_type(res.base_type_id);
			uint32_t bufferSize = (uint32_t)compiler.get_declared_struct_size(bufferType);
			uint32_t binding = compiler.get_decoration(res.id, spv::DecorationBinding);
			uint32_t memberCount = (uint32_t)bufferType.member_types.size();

			AR_CORE_TRACE_TAG("REFLECT", "\tName: {0}", bufferName);
			AR_CORE_TRACE_TAG("REFLECT", "\t   Size: {0}", bufferSize);
			AR_CORE_TRACE_TAG("REFLECT", "\t   Binding: {0}", binding);
			AR_CORE_TRACE_TAG("REFLECT", "\t   Member Count: {0}", memberCount);
			AR_CORE_TRACE_TAG("REFLECT", "------------------------------");
		}

		AR_CORE_TRACE_TAG("REFLECT", "Push Constant Buffers:");
		uint32_t attributeOffset = 0;
		for (const spirv_cross::Resource& res : resources.push_constant_buffers)
		{
			const std::string& bufferName = res.name;
			const spirv_cross::SPIRType& bufferType = compiler.get_type(res.base_type_id);
			uint32_t bufferSize = (uint32_t)compiler.get_declared_struct_size(bufferType);
			uint32_t memberCount = (uint32_t)bufferType.member_types.size();
			uint32_t location = compiler.get_decoration(res.id, spv::DecorationLocation);

			AR_CORE_TRACE_TAG("REFLECT", "\tName: {0}", bufferName);
			AR_CORE_TRACE_TAG("REFLECT", "\t   Size: {0}", bufferSize);
			AR_CORE_TRACE_TAG("REFLECT", "\t   Location: {0}", location);
			AR_CORE_TRACE_TAG("REFLECT", "\t   Member Count: {0}", memberCount);

			// Vertex Shader push_constant buffer which will be specific for the renderer!
			//if (bufferName == "u_Renderer")
			//	continue;

			// We create and insert a ShaderPushBuffer into the map to later on be used in the material to get the uniform location
			ShaderPushBuffer& buffer = m_Buffers[bufferName];
			buffer.Name = bufferName;
			buffer.Size = bufferSize - attributeOffset;

			for (uint32_t i = 0; i < memberCount; i++)
			{
				spirv_cross::SPIRType type = compiler.get_type(bufferType.member_types[i]);
				const std::string& memberName = compiler.get_member_name(bufferType.self, i);
				uint32_t memberSize = (uint32_t)compiler.get_declared_struct_member_size(bufferType, i);
				uint32_t memberOffset = compiler.type_struct_member_offset(bufferType, i) - attributeOffset;

				std::string UniformName = fmt::format("{}.{}", bufferName, memberName);
				buffer.Uniforms[UniformName] = ShaderUniform{ UniformName, Utils::SPIRTypeToShaderUniformType(type), memberSize, memberOffset };
			}
		}
		AR_CORE_TRACE_TAG("REFLECT", "------------------------------");

		//// TODO: REWRITE!!!!!!!!!!!!!!!!!!!!!!!
		//AR_CORE_TRACE_TAG("REFLECT", "Push Constant Buffers:");
		//// Currently this attribute offset var is useless since vulkan glsl supports only one push_constant.
		//// However if they ever remove that restriction this will be usefull to know the offset of each attribute in every push_constant block.
		//uint32_t attributeOffset = 0;
		//for (const spirv_cross::Resource& res : resources.push_constant_buffers)
		//{
		//	const std::string& bufferName = res.name; // Name
		//	const spirv_cross::SPIRType& bufferType = compiler.get_type(res.base_type_id); // Type
		//	uint32_t bufferSize = (uint32_t)compiler.get_declared_struct_size(bufferType); // Size
		//	uint32_t memberCount = (uint32_t)bufferType.member_types.size(); // Member count of the push_constant block
		//	uint32_t location = compiler.get_decoration(res.id, spv::DecorationLocation);

		//	// Since this is just a renderer special push_constant and will give us no valuable info, soo....
		//	if (bufferName == "u_Renderer")
		//		continue;

		//	ShaderPushBuffer& buffer = m_Buffers[bufferName];
		//	buffer.Name = bufferName;
		//	buffer.Size = bufferSize - attributeOffset;

		//	AR_CORE_TRACE_TAG("REFLECT", "\tName: {0}", bufferName);
		//	AR_CORE_TRACE_TAG("REFLECT", "\t   Size: {0}", bufferSize);
		//	AR_CORE_TRACE_TAG("REFLECT", "\t   Location: {0}", location);
		//	AR_CORE_TRACE_TAG("REFLECT", "\t   Member Count: {0}", memberCount);

		//	// Now we get the uniform/attributes that that push_constant block(buffer) contains...
		//	for (uint32_t i = 0; i < memberCount; i++)
		//	{
		//		spirv_cross::SPIRType type = compiler.get_type(bufferType.member_types[i]); // member type
		//		const std::string& memberName = compiler.get_member_name(bufferType.self, i); // member Name
		//		uint32_t size = (uint32_t)compiler.get_declared_struct_member_size(bufferType, i); // member Size
		//		uint32_t offset = compiler.type_struct_member_offset(bufferType, i) - attributeOffset; // member offset

		//		std::string uniformName = fmt::format("{}.{}", bufferName, memberName);
		//		buffer.Uniforms[uniformName] = ShaderUniform(uniformName, Utils::SPIRTypeToShaderUniformType(type), size, offset);
		//	}
		//}
		//AR_CORE_TRACE_TAG("REFLECT", "------------------------------");

		AR_CORE_TRACE_TAG("REFLECT", "Sampled Images:");
		int32_t sampler = 0;
		for (const spirv_cross::Resource& res : resources.sampled_images)
		{
			const spirv_cross::SPIRType& type = compiler.get_type(res.base_type_id);
			uint32_t binding = compiler.get_decoration(res.id, spv::DecorationBinding);
			const std::string& name = res.name;
			uint32_t dimension = type.image.dim;

			GLint location = glGetUniformLocation(m_ShaderID, name.c_str());
			AR_CORE_ASSERT(location != -1);
			m_Resources[name] = ShaderResourceDeclaration(name, binding, 1);
			glProgramUniform1i(m_ShaderID, location, binding); // Eventhough i think this might be useless since the shader sets the binding itself for the textures

			AR_CORE_TRACE_TAG("REFLECT", "\tName: {0}", name);
			AR_CORE_TRACE_TAG("REFLECT", "\t   Binding: {0}", binding);
		}
		AR_CORE_TRACE_TAG("REFLECT", "------------------------------");

		// TODO: Storage images.... however i wont get to that until ambient occlusion, and even then i might not use them!!!
	}

	std::unordered_map<uint32_t/*GLenum*/, std::string> Shader::SplitSource(const std::string& source)
	{ // Props to @TheCherno
		AR_PROFILE_FUNCTION();

		std::unordered_map<uint32_t/*GLenum*/, std::string> shaderSources;

		const char* typeIdentifier = "#pragma";
		size_t typeIdentifierLength = strlen(typeIdentifier);
		size_t pos = source.find(typeIdentifier, 0);

		while (pos != std::string::npos)
		{
			size_t eol = source.find_first_of("\r\n", pos); // End of shader type declaration line
			AR_CORE_ASSERT(eol != std::string::npos, "Syntax error!");

			size_t typeBegin = pos + typeIdentifierLength + 1; // Getting the type
			std::string type = source.substr(typeBegin, eol - typeBegin);
			AR_CORE_ASSERT(type == "vertex" || type == "fragment" || type == "geometry" || type == "compute", "Unknown Shader Type!");

			size_t nextLinePos = source.find_first_not_of("\r\n", eol); // Start of shader code

			pos = source.find(typeIdentifier, nextLinePos); // Start of next shader type declaration line

			uint32_t shaderType = Utils::ShaderTypeFromString(type);
			shaderSources[shaderType] = (pos == std::string::npos) ? source.substr(nextLinePos) : source.substr(nextLinePos, pos - nextLinePos);

			// Files containing compute shaders CAN NOT contain any other shader type!!
			if (shaderType == GL_COMPUTE_SHADER)
			{
				m_IsCompute = true;
				break;
			}
		}

		return shaderSources;
	}

	void Shader::Bind() const
	{
		AR_PROFILE_FUNCTION();

		glUseProgram(m_ShaderID);
	}

	void Shader::UnBind() const
	{
		glUseProgram(0);
	}

	const ShaderResourceDeclaration* Shader::GetShaderResource(const std::string& name) const
	{
		if (m_Resources.find(name) == m_Resources.end())
			return nullptr;

		return &(m_Resources.at(name));
	}

	int Shader::GetUniformLocation(const std::string& name) const
	{
		AR_PROFILE_FUNCTION();

		int location = glGetUniformLocation(m_ShaderID, name.c_str());
		if (location == -1)
			AR_CORE_WARN("Could not find uniform '{0}' in shader", name);

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