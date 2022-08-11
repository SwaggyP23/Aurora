#include "Aurorapch.h"
#include "ImageLoader.h"

#include <stb_image/stb_image.h>

namespace Aurora {

	namespace Utils {

		ImageData ImageLoader::m_ImageData;
		bool ImageLoader::m_Loading;

		ImageData ImageLoader::LoadImageFile(const std::string& filePath)
		{
			AR_PROFILE_FUNCTION();

			AR_CORE_ASSERT(std::filesystem::exists(filePath), "ImageLoader", "Path does not exist!");
			AR_CORE_ASSERT(!m_Loading, "ImageLoader", "You forgot to free an image after loading one somewhere!");

			m_Loading = true;
			m_ImageData.PixelData = (Byte*)stbi_load(filePath.c_str(), (int*)&m_ImageData.Width, (int*)&m_ImageData.Height, (int*)&m_ImageData.Channels, 0);

			return m_ImageData;
		}

		void ImageLoader::FreeImage()
		{
			AR_PROFILE_FUNCTION();

			AR_CORE_ASSERT(m_Loading, "ImageLoader", "Trying to call FreeImage without calling LoadImageFile which is not allowed!");

			m_Loading = false;
			stbi_image_free(m_ImageData.PixelData);
		}

		void ImageLoader::SetFlipVertically(bool boolean)
		{
			AR_PROFILE_FUNCTION();

			stbi_set_flip_vertically_on_load(boolean);
		}

	}

}