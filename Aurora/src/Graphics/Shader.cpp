#include "Aurorapch.h"
#include "Shader.h"

namespace Aurora {

	namespace Utils {

		enum class ShaderErrorType
		{
			None = -1,
			vertexShader,
			fragmentShader,
			geometryShader
		};

		static GLenum ShaderTypeFromString(const std::string& type)
		{
			if (type == "vertex")
				return GL_VERTEX_SHADER;
			if (type == "fragment")
				return GL_FRAGMENT_SHADER;
			if (type == "geometry") // Geometry shaders are currently useless since i am not using them
				return GL_GEOMETRY_SHADER;

			AR_CORE_ASSERT(false, "Unknown shader type!");
			return 0;
		}

		static ShaderErrorType ErrorTypeFromShaderType(GLenum shaderType)
		{
			switch (shaderType)
			{
			case GL_VERTEX_SHADER:		return ShaderErrorType::vertexShader;
			case GL_FRAGMENT_SHADER:	return ShaderErrorType::fragmentShader;
			case GL_GEOMETRY_SHADER:	return ShaderErrorType::geometryShader;
			}

			AR_CORE_ASSERT(false, "Unknown shader type!");
			return ShaderErrorType::None;
		}

		static void CheckShaderCompilation(GLuint shader, ShaderErrorType type)
		{
			GLint result;
			glGetShaderiv(shader, GL_COMPILE_STATUS, &result);
			if (!result) {
				GLint length;

				glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
				std::vector<char> errorMessage(length);

				glGetShaderInfoLog(shader, length, &length, &errorMessage[0]);

				if (type == ShaderErrorType::vertexShader)
					AR_CORE_ERROR("Failed to compile Vertex Shader!!");

				else if (type == ShaderErrorType::fragmentShader)
					AR_CORE_ERROR("Failed to compile Fragment Shader!!");

				AR_CORE_ERROR("Error message in function {0}: {1}", __FUNCTION__, &errorMessage[0]);

				glDeleteShader(shader);
			}
		}

		static void CheckProgramLinkage(GLuint program, const std::vector<GLuint>& shaderIDs)
		{
			GLint result;
			glGetProgramiv(program, GL_LINK_STATUS, &result);
			if (!result)
			{
				GLint length;

				glGetProgramiv(program, GL_INFO_LOG_LENGTH, &length);
				std::vector<char> errorMessage(length);

				glGetProgramInfoLog(program, length, NULL, &errorMessage[0]);
				AR_CORE_ERROR("Failed to link program!");
				AR_CORE_ERROR("Error message in function {0}: {1}", __FUNCTION__, &errorMessage[0]);

				glDeleteProgram(program);

				for (auto id : shaderIDs)
					glDeleteShader(id);
			}
		}

	}

	Ref<Shader> Shader::Create(const std::string& filepath)
	{
		return CreateRef<Shader>(filepath);
	}

	Shader::Shader(const std::string& filePath)
		: m_FilePath(filePath)
	{
		AR_PROFILE_FUNCTION();

		std::string shaderFullSource = Utils::FileReader::Get().ReadFile(filePath);

		auto shaderSplitSources = splitSource(shaderFullSource);

		m_ShaderID = createShaderProgram(shaderSplitSources);

		size_t lastSlash = filePath.find_last_of("/\\");
		lastSlash = lastSlash == std::string::npos ? 0 : lastSlash + 1;

		size_t lastDot = filePath.rfind('.');
		size_t count = lastDot == std::string::npos ? filePath.size() - lastSlash : lastDot - lastSlash;
		m_Name = filePath.substr(lastSlash, count);
	}

	std::unordered_map<GLenum, std::string> Shader::splitSource(const std::string& source)
	{ // Props to @TheCherno
		AR_PROFILE_FUNCTION();

		std::unordered_map<GLenum, std::string> shaderSources;

		const char* typeIdentifier = "#pragma";
		size_t typeIdentifierLength = strlen(typeIdentifier);
		size_t pos = source.find(typeIdentifier, 0);

		while (pos != std::string::npos)
		{
			size_t eol = source.find_first_of("\r\n", pos); // End of shader type declaration line

			size_t typeBegin = pos + typeIdentifierLength + 1; // Getting the type
			std::string type = source.substr(typeBegin, eol - typeBegin);

			size_t nextLinePos = source.find_first_not_of("\r\n", eol); // Start of shader code

			pos = source.find(typeIdentifier, nextLinePos); // Start of next shader type declaration line

			shaderSources[Utils::ShaderTypeFromString(type)] = (pos == std::string::npos) ? source.substr(nextLinePos) : source.substr(nextLinePos, pos - nextLinePos);
		}

		return shaderSources;
	}

	GLuint Shader::createShaderProgram(const std::unordered_map<GLenum, std::string>& shaderSources) const
	{
		AR_PROFILE_FUNCTION();

		GLuint program = glCreateProgram();
		std::vector<GLuint> ShaderIDs;
		ShaderIDs.reserve(shaderSources.size());
		for (const auto& [type, source] : shaderSources)
		{
			GLuint shader = ShaderIDs.emplace_back(glCreateShader(type));

			const char* shaderCStr = source.c_str();
			glShaderSource(shader, 1, &shaderCStr, NULL);
			glCompileShader(shader);

			Utils::CheckShaderCompilation(shader, Utils::ErrorTypeFromShaderType(type));

			glAttachShader(program, shader);
		}

		glLinkProgram(program);

		Utils::CheckProgramLinkage(program, ShaderIDs);

		glValidateProgram(program);
		for (auto id : ShaderIDs) {
			glDetachShader(program, id);
			glDeleteShader(id); // This flags the shader for deletion but is not deleted untill it is not linked to any other program, in our case that is directly here since it is already detached
		}

		AR_CORE_ASSERT(program, "Program is null!");

		return program;
	}

	Shader::~Shader()
	{
		AR_PROFILE_FUNCTION();

		glDeleteProgram(m_ShaderID);
	}

	void Shader::bind() const
	{
		AR_PROFILE_FUNCTION();

		glUseProgram(m_ShaderID);
	}

	void Shader::unBind() const
	{
		glUseProgram(0);
	}

	void Shader::setUniform1i(const GLchar* name, GLuint val) const
	{
		glUniform1i(getUniformLocation(name), val);
	}

	void Shader::setUniformArrayi(const GLchar* name, GLint* vals, uint32_t count) const
	{
		glUniform1iv(getUniformLocation(name), count, vals);
	}

	void Shader::setUniform1f(const GLchar* name, GLfloat val) const
	{
		glUniform1f(getUniformLocation(name), val);
	}

	void Shader::setUniform2f(const GLchar* name, const glm::vec2& vector) const
	{
		glUniform2f(getUniformLocation(name), vector.x, vector.y);
	}

	void Shader::setUniform3f(const GLchar* name, const glm::vec3& vector) const
	{
		glUniform3f(getUniformLocation(name), vector.x, vector.y, vector.z);
	}

	void Shader::setUniform4f(const GLchar* name, const glm::vec4& vector) const
	{
		glUniform4f(getUniformLocation(name), vector.x, vector.y, vector.z, vector.w);
	}

	void Shader::setUniformMat3(const GLchar* name, const glm::mat3& matrix) const
	{
		glUniformMatrix3fv(getUniformLocation(name), 1, GL_FALSE, glm::value_ptr(matrix));
	}

	void Shader::setUniformMat3(const GLchar* name, const float* matrix) const
	{
		glUniformMatrix3fv(getUniformLocation(name), 1, GL_FALSE, matrix);
	}

	void Shader::setUniformMat4(const GLchar* name, const glm::mat4& matrix) const
	{
		glUniformMatrix4fv(getUniformLocation(name), 1, GL_FALSE, glm::value_ptr(matrix));
	}

	void Shader::setUniformMat4(const GLchar* name, const float* matrix) const
	{
		glUniformMatrix4fv(getUniformLocation(name), 1, GL_FALSE, matrix);
	}

	GLint Shader::getUniformLocation(const std::string& name) const // To be instrumented
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

	void ShaderLibrary::Add(const std::string& name, const Ref<Shader>& shader)
	{
		AR_PROFILE_FUNCTION();

		AR_CORE_ASSERT(!Exists(name), "Shader already exists!");
		m_Shaders[name] = shader;
	}

	void ShaderLibrary::Add(const Ref<Shader>& shader)
	{
		const std::string& name = shader->getName();
		Add(name, shader);
	}

	Ref<Shader> ShaderLibrary::Load(const std::string& filepath)
	{
		auto shader = Shader::Create(filepath);
		Add(shader);
		return shader;
	}

	Ref<Shader> ShaderLibrary::Load(const std::string& name, const std::string& filepath)
	{
		auto shader = Shader::Create(filepath);
		Add(name, shader);
		return shader;
	}

	Ref<Shader> ShaderLibrary::Get(const std::string& name)
	{
		AR_CORE_ASSERT(Exists(name), "Shader not found!");
		return m_Shaders[name];
	}

	bool ShaderLibrary::Exists(const std::string& name) const // This always crashes
	{
		return m_Shaders.find(name) != m_Shaders.end();
	}

}