#pragma once

#include "Core/Base.h"

#include <string>
#include <fstream>

namespace Aurora {

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

		class Time
		{
		public:
			static float GetTime();
		};

	}

}