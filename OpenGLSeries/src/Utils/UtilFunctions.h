#pragma once

struct ShaderSources
{
	std::string VertexSource;
	std::string FragmentSource;
};

class FileReader
{
public:
	static FileReader& Get();
	static ShaderSources ReadShaderFile(const std::string& filePath);

private:
	FileReader();
	static std::ifstream m_Stream;

	enum class ShaderType
	{
		None = -1,
		Vertex,
		Fragment
	};
};
