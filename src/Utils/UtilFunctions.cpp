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
	m_Stream.open(filePath, std::ios::in);

	if (m_Stream.is_open()) {
		std::stringstream ss[2];
		std::string line;
		ShaderType type = ShaderType::None;

		while (std::getline(m_Stream, line))
		{
			if (line.find("shader") != std::string::npos) {
				if (line.find("vertex") != std::string::npos)
					type = ShaderType::Vertex;
				else if (line.find("fragment") != std::string::npos)
					type = ShaderType::Fragment;
			}
			else {
				ss[(int)type] << line << '\n';
			}
		}
		return { ss[0].str(), ss[1].str() };
	}
	else {
		std::cout << "File could not be opened!" << std::endl;
		return { };
	}
}