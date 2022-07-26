#pragma once

#include "Core/Base.h"

#include <string>
#include <fstream>

namespace Aurora {

	namespace Utils {

		class WindowsFileDialogs
		{
		public:
			// Returns empty strings if cancelled
			static std::string OpenFile(const char* filter);
			static std::string SaveFile(const char* filter);

		};

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