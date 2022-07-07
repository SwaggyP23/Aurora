#include "OGLpch.h"
#include "Texture.h"

namespace Utils {

	struct Formats
	{
		GLint InternalFormat = 0;
		GLenum DataFormat = 0;
	};

	Formats getFormatsFromChannels(int channels)
	{
		GLint internalFormat = 0;
		GLenum dataFormat = 0;
		if (channels == 4)
		{
			internalFormat = GL_RGBA8;
			dataFormat = GL_RGBA;
		}
		else if (channels == 3)
		{
			internalFormat = GL_RGB8;
			dataFormat = GL_RGB;
		}

		CORE_ASSERT(internalFormat & dataFormat, "Format is not supported. Format is null!");

		return { internalFormat, dataFormat };
	}
}

Ref<Texture> Texture::Create(uint32_t width, uint32_t height)
{
	return CreateRef<Texture>(width, height);
}

Ref<Texture> Texture::Create(const std::string& filePath)
{
	return CreateRef<Texture>(filePath);
}

Texture::Texture(uint32_t width, uint32_t height)
	: m_Width(width), m_Height(height)
{
	PROFILE_FUNCTION();

	m_InternalFormat = GL_RGBA8;
	m_DataFormat = GL_RGBA;

	glCreateTextures(GL_TEXTURE_2D, 1, &m_TextID);

	glTextureStorage2D(m_TextID, 1, GL_RGBA8, m_Width, m_Height);

	setTextureWrapping(GL_REPEAT);
	setTextureFiltering(GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR);
}

Texture::Texture(const std::string& filePath)
	: m_Path(filePath), m_Width(0), m_Height(0), m_InternalFormat(0), m_DataFormat(0)
{
	PROFILE_FUNCTION();

	glCreateTextures(GL_TEXTURE_2D, 1, &m_TextID);
}

void Texture::setData(void* data, uint32_t size)
{
	PROFILE_FUNCTION();

#if _DEBUG
	uint32_t bitsPerChan = m_DataFormat == GL_RGBA ? 4 : 3;
	CORE_ASSERT(size == m_Width * m_Height * bitsPerChan, "Data must be an entire texture!");
#endif
	glTextureSubImage2D(m_TextID, 0, 0, 0, m_Width, m_Height, m_DataFormat, GL_UNSIGNED_BYTE, data);
}

Texture::~Texture()
{
	PROFILE_FUNCTION();

	glDeleteTextures(1, &m_TextID);
}

void Texture::setTextureWrapping(GLenum wrapMode) const
{
	PROFILE_FUNCTION();

	glTextureParameteri(m_TextID, GL_TEXTURE_WRAP_S, wrapMode);
	glTextureParameteri(m_TextID, GL_TEXTURE_WRAP_T, wrapMode);
}

void Texture::setTextureFiltering(GLenum minFilter, GLenum magFilter) const
{
	PROFILE_FUNCTION();

	glTextureParameteri(m_TextID, GL_TEXTURE_MIN_FILTER, minFilter);
	glTextureParameteri(m_TextID, GL_TEXTURE_MAG_FILTER, magFilter);
}

void Texture::flipTextureVertically(bool state)
{
	Utils::ImageLoader::Get().setFlipVertically(state);
}

void Texture::loadTextureData()
{
	PROFILE_FUNCTION();

	// load image
	const char* path = m_Path.c_str();

	Utils::ImageLoader::Get().LoadImageFile(path);

	m_Width = Utils::ImageLoader::Get().getWidth();
	m_Height = Utils::ImageLoader::Get().getHeight();

	if (Utils::ImageLoader::Get().getData())
	{
		PROFILE_SCOPE("Texture Storage! -- Texture::loadTextureData()!");

		int channels = Utils::ImageLoader::Get().getChannels();
		CORE_LOG_WARN("Number of channels for texture {0} is: {1}", m_Path, channels);

		Utils::Formats texFormat = Utils::getFormatsFromChannels(channels);
		m_InternalFormat = texFormat.InternalFormat;
		m_DataFormat = texFormat.DataFormat;

		CORE_ASSERT(m_InternalFormat && m_DataFormat, "Formats are not set!");

		glTextureStorage2D(m_TextID, 5, m_InternalFormat, m_Width, m_Height); // level is number of mipmaps
		glTextureSubImage2D(m_TextID, 0, 0, 0, m_Width, m_Height, m_DataFormat, GL_UNSIGNED_BYTE, Utils::ImageLoader::Get().getData());
		// the first 0 is the index of the first level/mipmap

		glGenerateTextureMipmap(m_TextID);
	}
	else
		CORE_LOG_ERROR("Failed to load Texture! {0}", m_Path);

	Utils::ImageLoader::Get().FreeImage();
}

void Texture::bind(uint32_t slot) const
{
	PROFILE_FUNCTION();

	glBindTextureUnit(slot, m_TextID);
	
}

void Texture::unBind(/*uint32_t slot*/) const
{
	PROFILE_FUNCTION();

	//glBindTextureUnit(slot, 0); // This throws OpenGL error 1282, need to take a look at the specification
	glBindTexture(GL_TEXTURE_2D, 0);
}