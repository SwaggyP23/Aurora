#pragma once

#include "Core/Base.h"

#include <string>
#include <fstream>

namespace Aurora::Utils {

#ifdef AURORA_PLATFORM_WINDOWS
	// TODO: Add other platform support
	class WindowsFileDialogs
	{
	public:
		// Returns empty strings if cancelled
		[[nodiscard]] static std::filesystem::path OpenFileDialog(const char* filter);
		[[nodiscard]] static std::filesystem::path SaveFileDialog(const char* filter);

	};

#endif

	class FileIO
	{
	public:
		[[nodiscard]] static std::string ReadTextFile(const std::filesystem::path& filePath);
		static void WriteToFile(const std::filesystem::path& filePath, const void* buffer, size_t typeSize, size_t size);

	};

	class Time
	{
	public:
		[[nodiscard]] static float GetTime();
	};

	class StringUtils
	{
	public:
		[[nodiscard]] static std::string& ToLower(std::string& string);
		[[nodiscard]] static std::string& ToUpper(std::string& string);

	};

}