#pragma once

// TODO: Should depricate since this invalidates the concept of an Image class and a Texture class

namespace Aurora {

	namespace Utils {

		struct ImageData
		{
			Byte* PixelData = nullptr;
			uint32_t Width = 0;
			uint32_t Height = 0;
			uint8_t Channels = 0;
		};

		class ImageLoader
		{
		public:
			static ImageData LoadHDRImageFile(const std::string& filePath);
			static ImageData LoadImageFile(const std::string& filePath);
			static bool WriteDataToPNGImage(const std::filesystem::path& filePath, const void* data, uint32_t width, uint32_t height, uint32_t channels, bool flip = false);
			static bool WriteDataToTGAImage(const std::filesystem::path& filePath, const void* data, uint32_t width, uint32_t height, uint32_t channels, bool flip = false);
			static bool WriteDataToBMPImage(const std::filesystem::path& filePath, const void* data, uint32_t width, uint32_t height, uint32_t channels, bool flip = false);
			static bool WriteDataToHDRImage(const std::filesystem::path& filePath, const float* data, uint32_t width, uint32_t height, uint32_t channels, bool flip = false);
			static void FreeImage(void* data);

			// This is to be used before calling LoadImageFile()!
			static void SetFlipVertically(bool boolean);

		private:
			static bool m_Loading;

		};

	}

}