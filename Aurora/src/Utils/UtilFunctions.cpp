#include "Aurorapch.h"
#include "UtilFunctions.h"

#include <glfw/glfw3.h>

namespace Aurora {

	namespace Utils {

		std::ifstream FileReader::m_Stream;

		FileReader::FileReader()
		{
		}

		FileReader& FileReader::Get()
		{
			static FileReader s_Instance;
			return s_Instance;
		}

		std::string FileReader::ReadFile(const std::string& filePath)
		{
			AR_PROFILE_FUNCTION();

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
					AR_CORE_ERROR("Could not read from file {0}", filePath);
				}
			}
			else
			{
				AR_CORE_ASSERT(false, "Could not open file {0}", filePath);
			}

			return result;
		}

		float Time::GetTime()
		{
			return (float)glfwGetTime();
		}

	}

}