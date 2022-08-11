#pragma once

namespace Aurora {

	namespace Utils {

		struct ImageData
		{
			Byte* PixelData = nullptr;
			uint32_t Width = 0;
			uint32_t Height = 0;
			uint16_t Channels = 0;
		};

		class ImageLoader
		{
		public:
			static ImageData LoadImageFile(const std::string& filePath);
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