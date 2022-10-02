#include "Aurorapch.h"
#include "ImageLoader.h"

#include <stb_image/stb_image.h>
#include <stb_image_writer/stb_image_write.h>

namespace Aurora {

	namespace Utils {

		bool ImageLoader::m_Loading;

		ImageData ImageLoader::LoadHDRImageFile(const std::string& filePath)
		{
			AR_CORE_CHECK(std::filesystem::exists(filePath), "Path deos not exist!");

			if (!stbi_is_hdr(filePath.c_str()))
				return {};

			AR_CORE_CHECK(!m_Loading, "You forgot to free an image after loading one somewhere!");

			ImageData ret;

			m_Loading = true;
			ret.PixelData = (Byte*)stbi_loadf(filePath.c_str(), (int*)&ret.Width, (int*)&ret.Height, (int*)&ret.Channels, 0);
		
			return ret;
		}

		ImageData ImageLoader::LoadImageFile(const std::string& filePath)
		{
			AR_CORE_CHECK(std::filesystem::exists(filePath), "Path does not exist!");
			AR_CORE_CHECK(!m_Loading, "You forgot to free an image after loading one somewhere!");

			ImageData ret;

			m_Loading = true;
			ret.PixelData = (Byte*)stbi_load(filePath.c_str(), (int*)&ret.Width, (int*)&ret.Height, (int*)&ret.Channels, 0);

			return ret;
		}

		bool ImageLoader::WriteDataToPNGImage(const std::filesystem::path& filePath, const void* data, uint32_t width, uint32_t height, uint32_t channels, bool flip)
		{
			AR_CORE_ASSERT(filePath.extension() == ".png");

			if (flip)
			{
				stbi__vertical_flip((void*)data, width, height, channels * sizeof(uint8_t));
			}

			if (stbi_write_png(filePath.string().c_str(), width, height, channels, data, width * channels))
				return true;

			return false;
		}

		bool ImageLoader::WriteDataToTGAImage(const std::filesystem::path& filePath, const void* data, uint32_t width, uint32_t height, uint32_t channels, bool flip)
		{
			AR_CORE_ASSERT(filePath.extension() == ".tga");

			if (flip)
			{
				stbi__vertical_flip((void*)data, width, height, channels * sizeof(uint8_t));
			}

			if (stbi_write_tga(filePath.string().c_str(), width, height, channels, data))
				return true;

			return false;
		}

		bool ImageLoader::WriteDataToBMPImage(const std::filesystem::path& filePath, const void* data, uint32_t width, uint32_t height, uint32_t channels, bool flip)
		{
			AR_CORE_ASSERT(filePath.extension() == ".bmp");

			if (flip)
			{
				stbi__vertical_flip((void*)data, width, height, channels * sizeof(uint8_t));
			}

			if (stbi_write_bmp(filePath.string().c_str(), width, height, channels, data))
				return true;

			return false;
		}

		bool ImageLoader::WriteDataToHDRImage(const std::filesystem::path& filePath, const float* data, uint32_t width, uint32_t height, uint32_t channels, bool flip)
		{
			AR_CORE_ASSERT(filePath.extension() == ".hdr");

			if (flip)
			{
				stbi__vertical_flip((void*)data, width, height, channels * sizeof(uint8_t));
			}

			if (stbi_write_hdr(filePath.string().c_str(), width, height, channels, data))
				return true;

			return false;
		}

		void ImageLoader::FreeImage(void* data)
		{
			AR_CORE_CHECK(m_Loading, "Trying to call FreeImage without calling LoadImageFile which is not allowed!");

			m_Loading = false;
			stbi_image_free(data);
		}

		void ImageLoader::SetFlipVertically(bool boolean)
		{
			stbi_set_flip_vertically_on_load(boolean);
		}

	}

}