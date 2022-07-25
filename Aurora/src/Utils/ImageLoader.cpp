#include "Aurorapch.h"
#include "ImageLoader.h"

namespace Aurora {

	namespace Utils {

		unsigned char* ImageLoader::m_Data;
		uint32_t ImageLoader::m_Width, ImageLoader::m_Height, ImageLoader::m_Channels;

		ImageLoader& ImageLoader::Get()
		{
			static ImageLoader s_Instance;

			return s_Instance;
		}

		void ImageLoader::LoadImageFile(const std::string& filePath)
		{
			AR_PROFILE_FUNCTION();

			m_Data = (uint8_t*)stbi_load(filePath.c_str(), (int*)&m_Width, (int*)&m_Height, (int*)&m_Channels, 0);
		}

		void ImageLoader::FreeImage()
		{
			AR_PROFILE_FUNCTION();

			stbi_image_free(m_Data);
		}

		void ImageLoader::SetFlipVertically(bool boolean)
		{
			AR_PROFILE_FUNCTION();

			stbi_set_flip_vertically_on_load(boolean);
		}

	}

}