#pragma once

#include "OGLpch.h"

class FileReader
{
public:
	static FileReader& Get();
	static std::string ReadFile(const std::string& filePath);

private:
	FileReader();
	static std::ifstream m_Stream;

};
