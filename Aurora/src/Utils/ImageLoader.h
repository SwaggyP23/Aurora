#pragma once

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
			static ImageData LoadImageFile(const std::string& filePath);
			static bool WriteDataToPNGImage(const std::string& filePath, const void* data, uint32_t width, uint32_t height, uint32_t channels, bool flip = false);
			static void FreeImage();

			// This is to be used before calling LoadImageFile()!
			static void SetFlipVertically(bool boolean);

			static inline ImageData GetImageData() { return m_ImageData; }

		private:
			static ImageData m_ImageData;
			static bool m_Loading;

		};

	}

}