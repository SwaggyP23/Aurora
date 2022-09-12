#pragma once

#include "Core/Base.h"

#include <string>
#include <fstream>

namespace Aurora {

	namespace Utils {

#ifdef AURORA_PLATFORM_WINDOWS
		// TODO: Add other platform support
		class WindowsFileDialogs
		{
		public:
			// Returns empty strings if cancelled
			static std::filesystem::path OpenFileDialog(const char* filter);
			static std::filesystem::path SaveFileDialog(const char* filter);

		};

#endif

		class FileIO
		{
		public:
			static std::string ReadTextFile(const std::filesystem::path& filePath);
			static void WriteToFile(const std::filesystem::path& filePath, const void* buffer, size_t typeSize, size_t size);

		};

		class Time
		{
		public:
			static float GetTime();
		};

		class StringUtils
		{
		public:
			static std::string& ToLower(std::string& string);
			static std::string& ToUpper(std::string& string);

		};

	}

}