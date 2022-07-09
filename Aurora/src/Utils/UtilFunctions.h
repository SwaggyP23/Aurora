#pragma once

#include "Aurora.h"

namespace Utils {

	class FileReader
	{
	public:
		static FileReader& Get();
		static std::string ReadFile(const std::string& filePath);

	private:
		FileReader();
		static std::ifstream m_Stream;

	};

}