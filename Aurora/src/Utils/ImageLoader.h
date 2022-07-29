#pragma once

#include <string>
#include <stb_image/stb_image.h>

namespace Aurora {

	namespace Utils {

		class ImageLoader
		{
		public:
			static ImageLoader& Get();
			static void LoadImageFile(const std::string& filePath);
			static void FreeImage();
			static void SetFlipVertically(bool boolean);

			inline unsigned char* GetData() const { return m_Data; }
			inline uint32_t GetWidth() const { return m_Width; }
			inline uint32_t GetHeight() const { return m_Height; }
			inline uint32_t GetChannels() const { return m_Channels; }

		private:
			ImageLoader() = default;
			static uint8_t* m_Data;
			static uint32_t m_Width, m_Height, m_Channels;

		};

	}

}