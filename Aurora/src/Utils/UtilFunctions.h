#pragma once

#include "Core/Base.h"
#include "Core/Buffer.h"

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
		static Buffer ReadBytes(const std::filesystem::path& filePath);
		static std::string ReadTextFile(const std::filesystem::path& filePath);
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
		static std::string ToLower(const std::string_view& string);
		static std::string ToUpper(const std::string_view& string);
		static std::vector<std::string> SplitString(const std::string_view string, const std::string_view& delimiters);

	};

}