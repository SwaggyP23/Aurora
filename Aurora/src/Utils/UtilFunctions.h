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
			static std::string ReadTextFile(const std::string& filePath);

		private:
			static std::ifstream m_Stream;

		};

		class Time
		{
		public:
			static float GetTime();
		};

	}

}