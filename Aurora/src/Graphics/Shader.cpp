#include "Aurorapch.h"
#include "Shader.h"

#include "Utils/UtilFunctions.h"

#include <glad/glad.h>

#include <shaderc/shaderc.hpp>
#include <spirv_cross/spirv_glsl.hpp>

namespace Aurora {

	std::vector<Ref<Shader>> Shader::s_Shaders;

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

	}

	///////////////////////////
	/// Shader Uniform!!!!!!!!!
	///////////////////////////

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

	///////////////////////////
	/// Shader!!!!!!!!!!!!!!!!!
	///////////////////////////

	size_t Shader::GetHash() const
	{
		return std::hash<std::string>{}(m_AssetPath);
	}

	Ref<Shader> Shader::Create(const std::string& filepath, bool forceCompile)
	{
		return CreateRef<Shader>(filepath, forceCompile);
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
			std::string shaderFullSource = Utils::FileReader::ReadTextFile(filePath);
			Load(shaderFullSource, forceCompile);
		}
	}

	Shader::~Shader()
	{
		AR_PROFILE_FUNCTION();

		glDeleteProgram(m_ShaderID);
	}

	void Shader::Load(const std::string& source, bool forceCompile)
	{
		m_ShaderSource = SplitSource(source);

		Utils::CreateCacheDirIfNeeded();
	
		{
			Timer timer;
			CompileOrGetVulkanBinary(m_ShaderSource);
			CompileOrGetOpenGLBinary();
			CreateProgram();
			AR_CORE_WARN_TAG("Shader", "Shader creation took {0}ms", timer.ElapsedMillis());
		}
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

	void Shader::CompileOrGetVulkanBinary(const std::unordered_map<uint32_t, std::string>& shaderSources, bool forceCompile)
	{
		shaderc::Compiler compiler;
		shaderc::CompileOptions options;

		options.SetTargetEnvironment(shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_3);
		options.AddMacroDefinition("OPENGL");
		const bool optimize = true;
		if (optimize)
			options.SetOptimizationLevel(shaderc_optimization_level_performance);

		std::filesystem::path cacheDir = Utils::GetCacheDirectory();

		m_VulkanSPIRV.clear();

		for (const auto& [type, source] : shaderSources)
		{
			std::filesystem::path shaderFilePath = m_AssetPath;
			std::filesystem::path cachedPath = cacheDir / (shaderFilePath.filename().string() + GLShaderTypeCachedVulkanFileExtension(type));
			std::string p = cachedPath.string();

			FILE* f;
			fopen_s(&f, p.c_str(), "rb"); // read binary
			if (f && !forceCompile)
			{
				fseek(f, 0, SEEK_END);
				uint64_t size = ftell(f);
				fseek(f, 0, SEEK_SET);
				m_VulkanSPIRV[type].resize(size / sizeof(uint32_t));
				fread(m_VulkanSPIRV[type].data(), sizeof(uint32_t), size, f);
				fclose(f);
			}
			else
			{
				shaderc::SpvCompilationResult result = compiler.CompileGlslToSpv(source, GLShaderTypeToShaderC(type), m_AssetPath.c_str(), options);
				if (result.GetCompilationStatus() != shaderc_compilation_status_success)
				{
					AR_CORE_ERROR_TAG("ShaderC Compilation", result.GetErrorMessage());
					AR_CORE_ASSERT(false);
				}

				m_VulkanSPIRV[type] = std::vector<uint32_t>(result.cbegin(), result.cend());

				FILE* f;
				fopen_s(&f, p.c_str(), "wb"); // write binary
				if (f)
				{
					fwrite(m_VulkanSPIRV[type].data(), sizeof(uint32_t), m_VulkanSPIRV[type].size(), f);
					fclose(f);
				}
			}
		}

		for (auto&& [type, data] : m_VulkanSPIRV)
			Reflect(type, data);
	}

	void Shader::CompileOrGetOpenGLBinary(bool forceCompile)
	{
		shaderc::Compiler compiler;
		shaderc::CompileOptions options;

		options.SetTargetEnvironment(shaderc_target_env_opengl_compat, shaderc_env_version_opengl_4_5);
		const bool optimize = true;
		if (optimize)
			options.SetOptimizationLevel(shaderc_optimization_level_performance);

		std::filesystem::path cacheDir = Utils::GetCacheDirectory();

		m_OpenGLSPIRV.clear();
		for (const auto& [type, spirv] : m_VulkanSPIRV)
		{
			std::filesystem::path shaderFilePath = m_AssetPath;
			std::filesystem::path cachedPath = cacheDir / (shaderFilePath.filename().string() + GLShaderTypeCachedOpenGLFileExtension(type));
			std::string p = cachedPath.string();

			FILE* f;
			fopen_s(&f, p.c_str(), "rb"); // read binary
			if (f && !forceCompile)
			{
				fseek(f, 0, SEEK_END);
				uint64_t size = ftell(f);
				fseek(f, 0, SEEK_SET);
				m_OpenGLSPIRV[type].resize(size / sizeof(uint32_t));
				fread(m_OpenGLSPIRV[type].data(), sizeof(uint32_t), size, f);
				fclose(f);
			}
			else
			{
				spirv_cross::CompilerGLSL glslCompiler(spirv);
				m_ShaderSource[type] = glslCompiler.compile();

				shaderc::SpvCompilationResult result = compiler.CompileGlslToSpv(m_ShaderSource[type], GLShaderTypeToShaderC(type), m_AssetPath.c_str());
				if (result.GetCompilationStatus() != shaderc_compilation_status_success)
				{
					AR_CORE_ERROR_TAG("ShaderC Compilation", result.GetErrorMessage());
					AR_CORE_ASSERT(false);
				}

				m_OpenGLSPIRV[type] = std::vector<uint32_t>(result.cbegin(), result.cend());

				FILE* f;
				fopen_s(&f, p.c_str(), "wb"); // write binary
				if (f)
				{
					fwrite(m_OpenGLSPIRV[type].data(), sizeof(uint32_t), m_OpenGLSPIRV[type].size(), f);
					fclose(f);
				}
			}
		}
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
			AR_CORE_ERROR_TAG("Program Linkage", "Shader linking failed! {0}:\n\t{1}", m_AssetPath, std::string(message));

			glDeleteProgram(program);

			for (auto id : shaderIDs)
				glDeleteShader(id);
		}

		for (auto id : shaderIDs)
		{
			glDetachShader(program, id);
			glDeleteShader(id);
		}

		m_ShaderID = program;
	}

	void Shader::Reflect(uint32_t/*GLenum*/type, const std::vector<uint32_t>& shaderData)
	{
		spirv_cross::CompilerGLSL compiler(shaderData);
		spirv_cross::ShaderResources resources = compiler.get_shader_resources();

		AR_CORE_TRACE_TAG("REFLECT", "{0} - {1}", GLShaderTypeToString(type), m_AssetPath);
		AR_CORE_TRACE_TAG("REFLECT", "\t{0} Uniform Buffers", resources.uniform_buffers.size());
		AR_CORE_TRACE_TAG("REFLECT", "\t{0} Resources", resources.sampled_images.size());

		AR_CORE_TRACE_TAG("REFLECT", "Uniform Buffers:");
		for (const auto& res : resources.uniform_buffers)
		{
			const auto& bufferType = compiler.get_type(res.base_type_id);
			uint32_t bufferSize = (uint32_t)compiler.get_declared_struct_size(bufferType);
			uint32_t binding = compiler.get_decoration(res.id, spv::DecorationBinding);
			int memberCount = (int)bufferType.member_types.size();

			AR_CORE_TRACE_TAG("REFLECT", "\tName: {0}", res.name);
			AR_CORE_TRACE_TAG("REFLECT", "\tSize: {0}", bufferSize);
			AR_CORE_TRACE_TAG("REFLECT", "\tBinding: {0}", binding);
			AR_CORE_TRACE_TAG("REFLECT", "\tMember Count: {0}", memberCount);
		}
	}

	//void Shader::CompileOrGetOpenGLBinary(const std::unordered_map<uint32_t, std::vector<uint32_t>>& vulkanBinaries, bool forceCompile)
	//{
	//	if (m_ShaderID)
	//		glDeleteProgram(m_ShaderID);

	//	m_ShaderID = glCreateProgram();

	//	std::vector<GLuint> shaderIDs;
	//	shaderIDs.reserve(vulkanBinaries.size());

	//	std::filesystem::path cacheDir = Utils::GetCacheDirectory();

	//	m_ConstantBufferOffset = 0;
	//	std::vector<std::vector<uint32_t>> shaderData;
	//	for (const auto& [type, binary] : vulkanBinaries)
	//	{
	//		shaderc::Compiler compiler;
	//		shaderc::CompileOptions options;
	//		options.SetTargetEnvironment(shaderc_target_env_opengl_compat, shaderc_env_version_opengl_4_5);

	//		{
	//			spirv_cross::CompilerGLSL glsl(binary);
	//			ParseConstantBuffer(glsl);

	//			std::filesystem::path p = m_AssetPath;
	//			std::filesystem::path path = cacheDir / (p.filename().string() + GLShaderTypeCachedOpenGLFileExtension(type));
	//			std::string cachedFilePath = path.string();

	//			std::vector<uint32_t>& shaderTypeData = shaderData.emplace_back();

	//			if (!forceCompile)
	//			{
	//				FILE* f;
	//				fopen_s(&f, cachedFilePath.c_str(), "rb");
	//				if (f)
	//				{
	//					fseek(f, 0, SEEK_END);
	//					uint64_t size = ftell(f);
	//					fseek(f, 0, SEEK_SET);
	//					shaderTypeData = std::vector<uint32_t>(size / sizeof(uint32_t));
	//					fread(shaderTypeData.data(), sizeof(uint32_t), shaderTypeData.size(), f);
	//					fclose(f);
	//				}
	//			}

	//			if (!shaderTypeData.size())
	//			{
	//				std::string source = glsl.compile();

	//				printf("==============================\n");
	//				printf("%s Shader:\n%s\n", GLShaderTypeToString(type), source.c_str());
	//				printf("==============================\n");

	//				shaderc::SpvCompilationResult result = compiler.CompileGlslToSpv(source, GLShaderTypeToShaderC(type), m_AssetPath.c_str(), options);
	//			
	//				if (result.GetCompilationStatus() != shaderc_compilation_status_success)
	//				{
	//					AR_CORE_ERROR_TAG("ShaderC Compilation", result.GetErrorMessage());
	//					AR_CORE_ASSERT(false);
	//				}

	//				shaderTypeData = std::vector<uint32_t>(result.cbegin(), result.cend());
	//				
	//				{
	//					std::filesystem::path p = m_AssetPath;
	//					std::filesystem::path path = cacheDir / (p.filename().string() + GLShaderTypeCachedOpenGLFileExtension(type));
	//					cachedFilePath = path.string();
	//					FILE* f;
	//					fopen_s(&f, cachedFilePath.c_str(), "wb");
	//					fwrite(shaderTypeData.data(), sizeof(uint32_t), shaderTypeData.size(), f);
	//					fclose(f);
	//				}
	//			}

	//			GLuint shaderID = glCreateShader(type);
	//			glShaderBinary(1, &shaderID, GL_SHADER_BINARY_FORMAT_SPIR_V, shaderTypeData.data(), (uint32_t)(shaderTypeData.size() * sizeof(uint32_t)));
	//			// Sets the entrypoint for the shader
	//			glSpecializeShader(shaderID, "main", 0, nullptr, nullptr);
	//			glAttachShader(m_ShaderID, shaderID);

	//			shaderIDs.emplace_back(shaderID);
	//		}
	//	}

	//	// Link shader and check linking errors and status...
	//	glLinkProgram(m_ShaderID);

	//	GLint link = 0;
	//	glGetProgramiv(m_ShaderID, GL_LINK_STATUS, &link);
	//	if (!link)
	//	{
	//		GLint length = 0;
	//		glGetProgramiv(m_ShaderID, GL_INFO_LOG_LENGTH, &length);
	//		char* message = (char*)alloca(length * sizeof(char));

	//		glGetProgramInfoLog(m_ShaderID, length, &length, message);
	//		AR_CORE_ERROR_TAG("Shader Linking", "Shader Linking failed ({0}): {1}", m_AssetPath, message);

	//		glDeleteProgram(m_ShaderID);

	//		for (auto ID : shaderIDs)
	//			glDeleteShader(ID);
	//	}

	//	for (auto ID : shaderIDs)
	//		glDetachShader(m_ShaderID, ID);

	//	// Get uniform Locations
	//	for (auto& [buffernName, buffer] : m_Buffers)
	//	{
	//		for (auto& [name, uniform] : buffer.Uniforms)
	//		{
	//			GLint location = glGetUniformLocation(m_ShaderID, name.c_str());
	//			if (location == -1)
	//				AR_CORE_WARN_TAG("Shader", "Could not find uniform location {0}", name);

	//			m_UniformLocations[name] = location;
	//		}
	//	}

	//	for (auto& shaderTypeData : shaderData)
	//		Reflect(shaderTypeData);
	//}

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

	void Shader::ParseConstantBuffer(const spirv_cross::CompilerGLSL& compiler)
	{
		spirv_cross::ShaderResources res = compiler.get_shader_resources();

		AR_CORE_TRACE_TAG("REFLECT", "Push Constant Buffers:");
		for (const auto& resource : res.push_constant_buffers)
		{
			const std::string& bufferName = resource.name;
			auto& bufferType = compiler.get_type(resource.base_type_id);
			auto bufferSize = (uint32_t)compiler.get_declared_struct_size(bufferType);

			if (bufferName.empty() || bufferName == "u_Renderer")
			{
				m_ConstantBufferOffset += bufferSize;
				continue;
			}

			auto location = compiler.get_decoration(resource.id, spv::DecorationLocation);
			uint32_t memberCount = (uint32_t)bufferType.member_types.size();

			auto& [Name, Size, uniforms] = m_Buffers[bufferName];
			Name = bufferName;
			Size = bufferSize - m_ConstantBufferOffset;
			for (uint32_t i = 0; i < memberCount; i++)
			{
				const auto& type = compiler.get_type(bufferType.member_types[i]);
				const auto& memberName = compiler.get_member_name(bufferType.self, i);
				const auto size = (uint32_t)compiler.get_declared_struct_member_size(bufferType, i);
				const auto offset = compiler.type_struct_member_offset(bufferType, i) - m_ConstantBufferOffset;

				std::string uniformName = fmt::format("{}.{}", bufferName, memberName);
				uniforms[uniformName] = ShaderUniform(uniformName, SPIRTypeToShaderUniformType(type), size, offset);
			}

			m_ConstantBufferOffset += bufferSize;
		}
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

			auto shaderType = Utils::ShaderTypeFromString(type);
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

	void Shader::SetUniform1i(const char* name, uint32_t val) const
	{
		glUniform1i(GetUniformLocation(name), val);
	}

	void Shader::SetUniformArrayi(const char* name, int* vals, uint32_t count) const
	{
		glUniform1iv(GetUniformLocation(name), count, vals);
	}

	void Shader::SetUniform1f(const char* name, float val) const
	{
		glUniform1f(GetUniformLocation(name), val);
	}

	void Shader::SetUniform2f(const char* name, const glm::vec2& vector) const
	{
		glUniform2f(GetUniformLocation(name), vector.x, vector.y);
	}

	void Shader::SetUniform3f(const char* name, const glm::vec3& vector) const
	{
		glUniform3f(GetUniformLocation(name), vector.x, vector.y, vector.z);
	}

	void Shader::SetUniform4f(const char* name, const glm::vec4& vector) const
	{
		glUniform4f(GetUniformLocation(name), vector.x, vector.y, vector.z, vector.w);
	}

	void Shader::SetUniformMat3(const char* name, const glm::mat3& matrix) const
	{
		glUniformMatrix3fv(GetUniformLocation(name), 1, GL_FALSE, glm::value_ptr(matrix));
	}

	void Shader::SetUniformMat3(const char* name, const float* matrix) const
	{
		glUniformMatrix3fv(GetUniformLocation(name), 1, GL_FALSE, matrix);
	}

	void Shader::SetUniformMat4(const char* name, const glm::mat4& matrix) const
	{
		glUniformMatrix4fv(GetUniformLocation(name), 1, GL_FALSE, glm::value_ptr(matrix));
	}

	void Shader::SetUniformMat4(const char* name, const float* matrix) const
	{
		glUniformMatrix4fv(GetUniformLocation(name), 1, GL_FALSE, matrix);
	}

	int Shader::GetUniformLocation(const std::string& name) const // To be instrumented
	{
		AR_PROFILE_FUNCTION();

		auto it = m_UniformLocations.find(name);
		if (it != m_UniformLocations.end())
			return it->second;
		// Directly returns the uniform location if it has been already cached, and if not adds it to the cache map.
		// This is faster by like flat 12 millisecond for setting 6 uniforms, which would drasticaly become more efficient when 
		// setting materials and meshes

		GLint location = glGetUniformLocation(m_ShaderID, name.c_str());
		m_UniformLocations[name] = location;

		return location;
		//return glGetUniformLocation(m_ShaderID, name.c_str()); // This is Without Caching!
	}

	//////////////////////////
	/// ShaderLibrary!!!!!!!!!
	//////////////////////////

	void ShaderLibrary::Add(const std::string& name, const Ref<Shader>& shader)
	{
		AR_PROFILE_FUNCTION();

		AR_CORE_ASSERT(!Exist(name), "Shader already exists!");
		m_Shaders[name] = shader;
	}

	void ShaderLibrary::Add(const Ref<Shader>& shader)
	{
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
		AR_CORE_ASSERT(Exist(name), "Shader not found!");
		return m_Shaders[name];
	}

	bool ShaderLibrary::Exist(const std::string& name) const // TODO: See wassup with this since it always crashes
	{
		return m_Shaders.find(name) != m_Shaders.end();
	}

}