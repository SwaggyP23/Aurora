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

Ref<Texture> Texture::Create(const std::string& filePath)
{
	return CreateRef<Texture>(filePath);
}

Texture::Texture(const std::string& filePath)
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
	glTextureParameteri(m_TextID, GL_TEXTURE_WRAP_S, wrapMode);
	glTextureParameteri(m_TextID, GL_TEXTURE_WRAP_T, wrapMode);
}

void Texture::setTextureFiltering(GLenum minFilter, GLenum magFilter) const
{
	glTextureParameteri(m_TextID, GL_TEXTURE_MIN_FILTER, minFilter);
	glTextureParameteri(m_TextID, GL_TEXTURE_MAG_FILTER, magFilter);
}

void Texture::flipTextureVertically(bool state)
{
	Utils::ImageLoader::Get().setFlipVertically(state);
}

void Texture::loadTextureData()
{
	// load image
	const char* path = m_Path.c_str();
	Utils::ImageLoader::Get().LoadImageFile(path);

	m_Width = Utils::ImageLoader::Get().getWidth();
	m_Height = Utils::ImageLoader::Get().getHeight();

	if (Utils::ImageLoader::Get().getData()) {

		int channels = Utils::ImageLoader::Get().getChannels();
		CORE_LOG_WARN("Number of channels for texture {0} is: {1}", m_Path, channels);

		Utils::Formats texFormat = Utils::getFormatsFromChannels(channels);

		glTextureStorage2D(m_TextID, 5, texFormat.InternalFormat, m_Width, m_Height); // level is number of mipmaps
		glTextureSubImage2D(m_TextID, 0, 0, 0, m_Width, m_Height, texFormat.DataFormat, GL_UNSIGNED_BYTE, Utils::ImageLoader::Get().getData());
		// the first 0 is the index of the first level/mipmap

		glGenerateTextureMipmap(m_TextID);
	}
	else {
		CORE_LOG_ERROR("Failed to load Texture!! {0}", m_Path);
	}

	Utils::ImageLoader::Get().FreeImage();
}

void Texture::bind(/*uint32_t slot*/) const
{
	glBindTexture(GL_TEXTURE_2D, m_TextID);
	//glBindTextureUnit(slot, m_TextID); // This is to be used instead of the vector loop that is binding the textures to slots
	// This should be used instead of that to bind each texture to its certain slot.
}

void Texture::unBind() const
{
	glBindTexture(GL_TEXTURE_2D, 0);
}