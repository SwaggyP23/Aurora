#include "OGLpch.h"
#include "UtilFunctions.h"

std::ifstream FileReader::m_Stream;

FileReader::FileReader()
{
}

FileReader& FileReader::Get()
{
	static FileReader s_Instance;
	return s_Instance;
}

ShaderSources FileReader::ReadShaderFile(const std::string& filePath)
{
	std::string result;
	std::ifstream in(filePath, std::ios::in | std::ios::binary); // ifstream closes itself due to RAII
	if (in)
	{
		in.seekg(0, std::ios::end);
		size_t size = in.tellg();
		if (size != -1)
		{
			result.resize(size);
			in.seekg(0, std::ios::beg);
			in.read(&result[0], size);
		}
		else
		{
			CORE_LOG_ERROR("Could not read from file '{0}'", filePath);
		}
	}
	else
	{
		CORE_LOG_ERROR("Could not open file '{0}'", filePath);
	}

	size_t ind = result.find("#shader fragment", 0);
	std::string vertex = result.substr(15, ind - 16);
	std::string fragment = result.substr(ind + 16, result.size() - 1);

	return { vertex, fragment };
}