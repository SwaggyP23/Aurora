#include "Aurorapch.h"
#include "UtilFunctions.h"

#include "Core/Application.h"

#ifdef AURORA_PLATFORM_WINDOWS
	#include <commdlg.h>
#endif

#include <glfw/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <glfw/glfw3native.h>

namespace Aurora {

	namespace Utils {

#ifdef AURORA_PLATFORM_WINDOWS

		// This is literally WinAPI boiler plate code
		std::filesystem::path WindowsFileDialogs::OpenFileDialog(const char* filter)
		{
			OPENFILENAMEA ofn; // common dialog box structure
			CHAR szFile[260] = { 0 }; // If using TCHAR macro

			// Initialize OPENFILENAMEA
			ZeroMemory(&ofn, sizeof(OPENFILENAMEA));
			ofn.lStructSize = sizeof(OPENFILENAMEA);
			ofn.hwndOwner = glfwGetWin32Window((GLFWwindow*)Application::GetApp().GetWindow().GetWindowPointer());
			ofn.lpstrFile = szFile;
			ofn.nMaxFile = sizeof(szFile);
			ofn.lpstrFilter = filter;
			ofn.nFilterIndex = 1;
			ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

			if (GetOpenFileNameA(&ofn) == TRUE)
			{
				std::string fp = ofn.lpstrFile;
				std::replace(fp.begin(), fp.end(), '\\', '/');
				return std::filesystem::path(fp);
			}
			
			return std::filesystem::path();
		}

		// This is literally WinAPI boiler plate code
		std::filesystem::path WindowsFileDialogs::SaveFileDialog(const char* filter)
		{
			OPENFILENAMEA ofn; // common dialog box structure
			CHAR szFile[260] = { 0 }; // If using TCHAR macro

			// Initialize OPENFILENAMEA
			ZeroMemory(&ofn, sizeof(OPENFILENAMEA));
			ofn.lStructSize = sizeof(OPENFILENAMEA);
			ofn.hwndOwner = glfwGetWin32Window((GLFWwindow*)Application::GetApp().GetWindow().GetWindowPointer());
			ofn.lpstrFile = szFile;
			ofn.nMaxFile = sizeof(szFile);
			ofn.lpstrFilter = filter;
			ofn.nFilterIndex = 1;
			ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

			if (GetSaveFileNameA(&ofn) == TRUE)
			{
				std::string fp = ofn.lpstrFile;
				std::replace(fp.begin(), fp.end(), '\\', '/');
				return std::filesystem::path(fp);
			}

			return std::filesystem::path();
		}

#endif

		Buffer FileIO::ReadBytes(const std::filesystem::path& filePath)
		{
			Buffer buffer;

			std::ifstream stream(filePath, std::ios::binary | std::ios::ate); // Goes directly to end

			if (stream.is_open())
			{
				uint64_t end = stream.tellg();
				stream.seekg(0, std::ios::beg);
				uint64_t size = end - stream.tellg();
				AR_CORE_ASSERT(size != 0);

				buffer.Allocate((size_t)size);
				stream.read((char*)buffer.Data, buffer.Size);
				stream.close();

				return buffer;
			}

			return {};
		}

		std::string FileIO::ReadTextFile(const std::filesystem::path& filePath)
		{
			std::string result;

			FILE* f;
			fopen_s(&f, filePath.string().c_str(), "rb");
			if (f)
			{
				fseek(f, 0, SEEK_END);
				uint64_t size = ftell(f);
				result.resize(size);
				fseek(f, 0, SEEK_SET);
				fread(&result[0], sizeof(char), size / sizeof(char), f);
				fclose(f);
			}
			else
			{
				AR_CORE_CRITICAL_TAG("FileIO", "Could not open file: {0}", filePath.string());
			}

			return result;
		}

		void FileIO::WriteToFile(const std::filesystem::path& filePath, const void* buffer, size_t typeSize, size_t size)
		{
			FILE* f;
			fopen_s(&f, filePath.string().c_str(), "wb");
			if (f)
			{
				fwrite(buffer, typeSize, size, f);
				fclose(f);
			}
			else
			{
				AR_CORE_CRITICAL_TAG("FileIO", "Could not open file: {0}", filePath.string());
			}
		}

		// Time...
		float Time::GetTime()
		{
			return (float)glfwGetTime();
		}

		// String stuff...
		std::string StringUtils::ToLower(const std::string_view& string)
		{
			std::string result;
			result.resize(string.size());
			for (int i = 0; i < string.size(); i++)
			{
				result[i] = std::tolower(string[i]);
			}

			return result;
		}

		std::string StringUtils::ToUpper(const std::string_view& string)
		{
			std::string result;
			result.resize(string.size());
			for (int i = 0; i < string.size(); i++)
			{
				result[i] = std::toupper(string[i]);
			}

			return result;
		}

		std::vector<std::string> StringUtils::SplitString(const std::string_view string, const std::string_view& delimiters)
		{
			size_t first = 0;
			std::vector<std::string> result;

			while (first <= string.size())
			{
				const size_t second = string.find_first_of(delimiters, first);

				if (first != second)
					result.emplace_back(string.substr(first, second - first));

				if (second == std::string::npos)
					break;

				first = second + 1;
			}

			return result;
		}

	}

}