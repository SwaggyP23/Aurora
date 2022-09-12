#include "Aurorapch.h"
#include "ImageLoader.h"

#include <stb_image/stb_image.h>
#include <stb_image_writer/stb_image_write.h>

namespace Aurora {

	namespace Utils {

		ImageData ImageLoader::m_ImageData;
		bool ImageLoader::m_Loading;

		ImageData ImageLoader::LoadImageFile(const std::string& filePath)
		{
			AR_PROFILE_FUNCTION();

			AR_CORE_ASSERT(std::filesystem::exists(filePath), "Path does not exist!");
			AR_CORE_ASSERT(!m_Loading, "You forgot to free an image after loading one somewhere!");

			m_Loading = true;
			m_ImageData.PixelData = (Byte*)stbi_load(filePath.c_str(), (int*)&m_ImageData.Width, (int*)&m_ImageData.Height, (int*)&m_ImageData.Channels, 0);

			return m_ImageData;
		}

		bool ImageLoader::WriteDataToPNGImage(const std::string& filePath, const void* data, uint32_t width, uint32_t height, uint32_t channels)
		{
			// Should Return a bool if it works
			// Should have an enum to select in what format to write the image (tga/png/jpg/hdr...)
			if (stbi_write_png(filePath.c_str(), width, height, channels, data, width * channels))
				return true;

			return false;
		}

		void ImageLoader::FreeImage()
		{
			AR_PROFILE_FUNCTION();

			AR_CORE_ASSERT(m_Loading, "Trying to call FreeImage without calling LoadImageFile which is not allowed!");

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