#include "Shader.h"
#include <vector>
#include "Logging/Log.h"

Shader::Shader(const std::string& filePath)
{
	ShaderSources ss = FileReader::Get().ReadShaderFile(filePath);

	m_VertexShaderCode = ss.VertexSource;
	m_FragmentShaderCode = ss.FragmentSource;

	m_ShaderID = createShaderProgram();
}

Shader::~Shader()
{
	glDeleteProgram(m_ShaderID);
}

void Shader::bind() const
{
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

void Shader::setUniformMat4(const GLchar* name, const glm::mat4& matrix) const
{
	glUniformMatrix4fv(getUniformLocation(name), 1, GL_FALSE, glm::value_ptr(matrix));
}

void Shader::setUniformMat4(const GLchar* name, const float* matrix) const
{
	glUniformMatrix4fv(getUniformLocation(name), 1, GL_FALSE, matrix);
}

GLint Shader::getUniformLocation(const GLchar* name) const
{
	return glGetUniformLocation(m_ShaderID, name);
}

GLuint Shader::createShaderProgram() const
{
	GLuint program = glCreateProgram();
	GLuint vs = glCreateShader(GL_VERTEX_SHADER);
	GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);

	const char* vsC = m_VertexShaderCode.c_str();
	glShaderSource(vs, 1, &vsC, NULL);
	glCompileShader(vs);

	CheckShaderCompilation(vs, ShaderErrorType::vertexShader);

	const char* fsC = m_FragmentShaderCode.c_str();
	glShaderSource(fs, 1, &fsC, NULL);
	glCompileShader(fs);

	CheckShaderCompilation(fs, ShaderErrorType::fragmentShader);

	glAttachShader(program, vs);
	glAttachShader(program, fs);
	glLinkProgram(program);

	glValidateProgram(program);
	glDeleteShader(vs);
	glDeleteShader(fs);

	return program;
}

void Shader::CheckShaderCompilation(GLuint shader, ShaderErrorType type) const
{
	GLint result;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &result);
	if (!result) {
		GLint length;

		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
		std::vector<char> errorMessage(length);

		glGetShaderInfoLog(shader, length, &length, &errorMessage[0]);

		if (type == ShaderErrorType::vertexShader)
			CORE_LOG_ERROR("Failed to compile Vertex Shader!!");

		else if (type == ShaderErrorType::fragmentShader)
			CORE_LOG_ERROR("Failed to compile Fragment Shader!!");

		CORE_LOG_ERROR("Error message in function {0}: {1}", __FUNCTION__, &errorMessage[0]);

		glDeleteShader(shader);
	}
}