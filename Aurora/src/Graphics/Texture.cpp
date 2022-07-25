#include "Aurorapch.h"
#include "Texture.h"

#include <glad/glad.h>

namespace Aurora {

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

			AR_CORE_ASSERT(internalFormat & dataFormat, "Format is not supported. Format is null!");

			return { internalFormat, dataFormat };
		}
	}

	static GLenum GLTypeFromTextureProperties(TextureProperties Type)
	{
		switch (Type)
		{
			case TextureProperties::Repeat:                      return GL_REPEAT;
			case TextureProperties::MirrorredRepeat:             return GL_MIRRORED_REPEAT;
			case TextureProperties::ClampToEdge:                 return GL_CLAMP_TO_EDGE;
			case TextureProperties::ClampToBorder:               return GL_CLAMP_TO_BORDER;
			case TextureProperties::Nearest:                     return GL_NEAREST;
			case TextureProperties::Linear:                      return GL_LINEAR;
			case TextureProperties::MipMap_NearestNearest:       return GL_NEAREST_MIPMAP_NEAREST;
			case TextureProperties::MipMap_LinearNearest:        return GL_LINEAR_MIPMAP_NEAREST;
			case TextureProperties::MipMap_NearestLinear:        return GL_NEAREST_MIPMAP_LINEAR;
			case TextureProperties::MipMap_LinearLinear:         return GL_LINEAR_MIPMAP_LINEAR;
		}

		AR_CORE_ASSERT(false, "Unkown texture Filtering/Wrapping type!");
		return 0;
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
		AR_PROFILE_FUNCTION();

		m_InternalFormat = GL_RGBA8;
		m_DataFormat = GL_RGBA;

		glCreateTextures(GL_TEXTURE_2D, 1, &m_TextID);

		glTextureStorage2D(m_TextID, 1, GL_RGBA8, m_Width, m_Height);

		SetTextureWrapping(TextureProperties::Repeat);
		SetTextureFiltering(TextureProperties::MipMap_LinearLinear, TextureProperties::Linear);
	}

	Texture::Texture(const std::string& filePath)
		: m_Path(filePath), m_Width(0), m_Height(0), m_InternalFormat(0), m_DataFormat(0)
	{
		AR_PROFILE_FUNCTION();

		glCreateTextures(GL_TEXTURE_2D, 1, &m_TextID);
	}

	void Texture::SetData(const void* data, uint32_t size)
	{
		AR_PROFILE_FUNCTION();

#ifdef AURORA_DEBUG
		uint32_t bitsPerChan = m_DataFormat == GL_RGBA ? 4 : 3;
		AR_CORE_ASSERT(size == m_Width * m_Height * bitsPerChan, "Data must be an entire texture!");
#endif
		glTextureSubImage2D(m_TextID, 0, 0, 0, m_Width, m_Height, m_DataFormat, GL_UNSIGNED_BYTE, data);
	}

	Texture::~Texture()
	{
		AR_PROFILE_FUNCTION();

		glDeleteTextures(1, &m_TextID);
	}

	void Texture::SetTextureWrapping(TextureProperties wrapMode) const
	{
		AR_PROFILE_FUNCTION();

		glTextureParameteri(m_TextID, GL_TEXTURE_WRAP_S, GLTypeFromTextureProperties(wrapMode));
		glTextureParameteri(m_TextID, GL_TEXTURE_WRAP_T, GLTypeFromTextureProperties(wrapMode));
	}

	void Texture::SetTextureFiltering(TextureProperties minFilter, TextureProperties magFilter) const
	{
		AR_PROFILE_FUNCTION();

		glTextureParameteri(m_TextID, GL_TEXTURE_MIN_FILTER, GLTypeFromTextureProperties(minFilter));
		glTextureParameteri(m_TextID, GL_TEXTURE_MAG_FILTER, GLTypeFromTextureProperties(magFilter));
	}

	void Texture::FlipTextureVertically(bool state)
	{
		Utils::ImageLoader::Get().SetFlipVertically(state);
	}

	void Texture::LoadTextureData()
	{
		AR_PROFILE_FUNCTION();

		// load image
		const char* path = m_Path.c_str();

		Utils::ImageLoader::Get().LoadImageFile(path);

		m_Width = Utils::ImageLoader::Get().GetWidth();
		m_Height = Utils::ImageLoader::Get().GetHeight();

		if (Utils::ImageLoader::Get().GetData())
		{
			AR_PROFILE_SCOPE("Texture Storage! Texture::loadTextureData()!");

			int channels = Utils::ImageLoader::Get().GetChannels();
			AR_CORE_WARN("Number of channels for texture {0} is: {1}", m_Path, channels);

			Utils::Formats texFormat = Utils::getFormatsFromChannels(channels);
			m_InternalFormat = texFormat.InternalFormat;
			m_DataFormat = texFormat.DataFormat;

			AR_CORE_ASSERT(m_InternalFormat && m_DataFormat, "Formats are not set!");

			glTextureStorage2D(m_TextID, 5, m_InternalFormat, m_Width, m_Height); // level is number of mipmaps
			glTextureSubImage2D(m_TextID, 0, 0, 0, m_Width, m_Height, m_DataFormat, GL_UNSIGNED_BYTE, Utils::ImageLoader::Get().GetData());
			// the first 0 is the index of the first level/mipmap

			glGenerateTextureMipmap(m_TextID);
		}
		else
			AR_CORE_ERROR("Failed to load Texture! {0}", m_Path);

		Utils::ImageLoader::Get().FreeImage();
	}

	void Texture::bind(uint32_t slot) const
	{
		AR_PROFILE_FUNCTION();

		glBindTextureUnit(slot, m_TextID);

	}

	void Texture::unBind(uint32_t slot) const
	{
		// This currently maybe works, however it throws OpenGL error 1282, need to take a look at the specification and if ever that error pops up check here
		glBindTextureUnit(slot, 0);
		//glBindTexture(GL_TEXTURE_2D, 0);
	}

}