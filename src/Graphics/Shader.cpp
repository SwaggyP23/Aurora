#include "Shader.h"
#include <iostream>
#include <vector>

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
			std::cout << "Failed to compile Vertex Shader!!\n";

		else if (type == ShaderErrorType::fragmentShader)
			std::cout << "Failed to compile Fragment Shader!!\n";

		std::cout << &errorMessage[0] << std::endl;

		glDeleteShader(shader);
	}
}