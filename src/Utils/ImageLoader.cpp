#include "ImageLoader.h"

unsigned char* ImageLoader::m_Data;
uint32_t ImageLoader::m_Width, ImageLoader::m_Height, ImageLoader::m_Channels;

ImageLoader& ImageLoader::Get()
{
	static ImageLoader s_Instance;

	return s_Instance;
}

void ImageLoader::LoadImageFile(const char* filePath)
{
	m_Data = (uint8_t*)stbi_load(filePath, (int*)&m_Width, (int*)&m_Height, (int*)&m_Channels, 0);
}

void ImageLoader::FreeImage()
{
	stbi_image_free(m_Data);
}

void ImageLoader::setFlipVertically(bool boolean)
{
	stbi_set_flip_vertically_on_load(boolean);
}