#pragma once

#include "Aurora.h"

namespace Aurora {

	namespace Utils {

		class ImageLoader
		{
		public:
			static ImageLoader& Get();
			static void LoadImageFile(const char* filePath);
			static void FreeImage();
			static void setFlipVertically(bool boolean);

			inline unsigned char* getData() const { return m_Data; }
			inline uint32_t getWidth() const { return m_Width; }
			inline uint32_t getHeight() const { return m_Height; }
			inline uint32_t getChannels() const { return m_Channels; }

		private:
			ImageLoader() = default;
			static uint8_t* m_Data;
			static uint32_t m_Width, m_Height, m_Channels;

		};

	}

}