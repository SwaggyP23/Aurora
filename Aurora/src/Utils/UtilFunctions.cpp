#include "Aurorapch.h"
#include "UtilFunctions.h"

#include "Core/Application.h"

#ifdef AR_PLATFORM_WINDOWS
	#include <commdlg.h>
#endif

#include <glfw/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <glfw/glfw3native.h>

namespace Aurora {

	namespace Utils {

#ifdef AR_PLATFORM_WINDOWS

		// This is literally WinAPI boiler plate code
		std::string WindowsFileDialogs::OpenFile(const char* filter)
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
				return ofn.lpstrFile;
			}
			
			return std::string();
		}

		// This is literally WinAPI boiler plate code
		std::string WindowsFileDialogs::SaveFile(const char* filter)
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
				return ofn.lpstrFile;
			}

			return std::string();
		}

#endif

		std::ifstream FileReader::m_Stream;

		std::string FileReader::ReadTextFile(const std::string& filePath)
		{
			AR_PROFILE_FUNCTION();

			AR_CORE_ASSERT(std::filesystem::exists(filePath), "[FileReader]: Filepath provided does not exist!");

			std::string result;
			std::ifstream in(filePath, std::ios::in | std::ios::binary); // ifstream closes itself due to RAII
			if (in.is_open())
			{
				in.seekg(0, std::ios::end);
				size_t size = in.tellg();
				if (size != -1)
				{
					result.resize(size);
					in.seekg(0, std::ios::beg);
					in.read(&result[0], size);
				}
				else
				{
					AR_CORE_ERROR("[FileReader]: Could not read from file {0}", filePath);
				}
			}
			else
			{
				AR_CORE_ASSERT(false, "[FileReader]: Could not open file {0}", filePath);
			}

			return result;
		}

		float Time::GetTime()
		{
			return (float)glfwGetTime();
		}

	}

}