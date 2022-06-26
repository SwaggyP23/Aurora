#include "../OGLpch.h"
#include "Texture.h"

Texture::Texture(std::string filePath)
	: m_Path(filePath), m_Width(0), m_Height(0)
{
	glGenTextures(1, &m_TextID);
}

Texture::~Texture()
{
	glDeleteTextures(1, &m_TextID);
}

void Texture::setTextureWrapping(GLenum wrapMode) const
{
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapMode);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapMode);
}

void Texture::setTextureFiltering(GLenum minFilter, GLenum magFilter) const
{
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter);
}

void Texture::loadTextureData(GLint internalFormat, GLenum format)
{
	// load image
	const char* path = m_Path.c_str();
	ImageLoader::Get().setFlipVertically(true);
	ImageLoader::Get().LoadImageFile(path);

	m_Width = ImageLoader::Get().getWidth();
	m_Height = ImageLoader::Get().getHeight();

	if (ImageLoader::Get().getData()) {
		glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, m_Width, m_Height, 0, format, GL_UNSIGNED_BYTE,
			ImageLoader::Get().getData());

		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else {
		CORE_LOG_ERROR("Failed to load Texture!!");
	}

	ImageLoader::Get().FreeImage();
}

void Texture::bind() const
{
	glBindTexture(GL_TEXTURE_2D, m_TextID);
}

void Texture::unBind() const
{
	glBindTexture(GL_TEXTURE_2D, 0);
}