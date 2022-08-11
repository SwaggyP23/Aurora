#include "Aurorapch.h"
#include "Texture.h"

#include "Utils/ImageLoader.h"

#include <glad/glad.h>

namespace Aurora {

	namespace Utils {

		struct Formats
		{
			GLint InternalFormat = 0;
			GLenum DataFormat = 0;
		};

		static Formats GetFormatsFromChannels(int channels)
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

			AR_CORE_ASSERT(internalFormat & dataFormat, "Texture", "Format is not supported.Format is null!");

			return { internalFormat, dataFormat };
		}
	}

	static GLenum GLFilterTypeFromTextureFilter(TextureFilter type)
	{
		switch (type)
		{
		    case TextureFilter::None:                        return GL_NONE;
		    case TextureFilter::Nearest:                     return GL_NEAREST;
		    case TextureFilter::Linear:                      return GL_LINEAR;
			case TextureFilter::MipMap_NearestNearest:       return GL_NEAREST_MIPMAP_NEAREST;
			case TextureFilter::MipMap_LinearNearest:        return GL_LINEAR_MIPMAP_NEAREST;
			case TextureFilter::MipMap_NearestLinear:        return GL_NEAREST_MIPMAP_LINEAR;
			case TextureFilter::MipMap_LinearLinear:         return GL_LINEAR_MIPMAP_LINEAR;
		}

		AR_CORE_ASSERT(false, "Texture", "Unknown Texture Filter!");
		return 0;
	}

	static GLenum GLWrapTypeFromTextureWrap(TextureWrap type)
	{
		switch (type)
		{
		    case TextureWrap::None:               return GL_NONE;
		    case TextureWrap::Repeat:             return GL_REPEAT;
		    case TextureWrap::MirrorredRepeat:    return GL_MIRRORED_REPEAT;
		    case TextureWrap::ClampToEdge:        return GL_CLAMP_TO_EDGE;
		    case TextureWrap::ClampToBorder:      return GL_CLAMP_TO_BORDER;
		}

		AR_CORE_ASSERT(false, "Texture", "Unknown Texture Wrap Mode!");
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

		glCreateTextures(GL_TEXTURE_2D, 1, &m_TextureID);

		glTextureStorage2D(m_TextureID, 1, GL_RGBA8, m_Width, m_Height);

		SetTextureWrapping(TextureWrap::Repeat);
		SetTextureFiltering(TextureFilter::MipMap_LinearLinear, TextureFilter::Linear);
	}

	Texture::Texture(const std::string& filePath)
		: m_Path(filePath), m_Width(0), m_Height(0), m_InternalFormat(0), m_DataFormat(0)
	{
		AR_PROFILE_FUNCTION();

		glCreateTextures(GL_TEXTURE_2D, 1, &m_TextureID);
	}

	void Texture::SetData(const void* data, uint32_t size)
	{
		AR_PROFILE_FUNCTION();

#ifdef AURORA_DEBUG
		uint32_t bitsPerChan = m_DataFormat == GL_RGBA ? 4 : 3;
		AR_CORE_ASSERT(size == m_Width * m_Height * bitsPerChan, "Texture", "Data must be an entire texture!");
#endif
		glTextureSubImage2D(m_TextureID, 0, 0, 0, m_Width, m_Height, m_DataFormat, GL_UNSIGNED_BYTE, data);
	}

	Texture::~Texture()
	{
		AR_PROFILE_FUNCTION();

		glDeleteTextures(1, &m_TextureID);
	}

	void Texture::SetTextureWrapping(TextureWrap wrapMode) const
	{
		AR_PROFILE_FUNCTION();

		glTextureParameteri(m_TextureID, GL_TEXTURE_WRAP_S, GLWrapTypeFromTextureWrap(wrapMode));
		glTextureParameteri(m_TextureID, GL_TEXTURE_WRAP_T, GLWrapTypeFromTextureWrap(wrapMode));
	}

	void Texture::SetTextureFiltering(TextureFilter minFilter, TextureFilter magFilter) const
	{
		AR_PROFILE_FUNCTION();

		glTextureParameteri(m_TextureID, GL_TEXTURE_MIN_FILTER, GLFilterTypeFromTextureFilter(minFilter));
		glTextureParameteri(m_TextureID, GL_TEXTURE_MAG_FILTER, GLFilterTypeFromTextureFilter(magFilter));
	}

	void Texture::FlipTextureVertically(bool state)
	{
		Utils::ImageLoader::SetFlipVertically(state);
	}

	void Texture::LoadTextureData()
	{
		AR_PROFILE_FUNCTION();

		// load image
		const char* path = m_Path.c_str();

		auto& imageData = Utils::ImageLoader::LoadImageFile(path);

		m_Width = imageData.Width;
		m_Height = imageData.Height;

		if (imageData.PixelData)
		{
			AR_PROFILE_SCOPE("Texture Storage! Texture::loadTextureData()");

			int channels = imageData.Channels;
			AR_CORE_WARN_TAG("Texture", "Number of channels for texture{0} is: {1}", m_Path, channels);

			Utils::Formats texFormat = Utils::GetFormatsFromChannels(channels);
			m_InternalFormat = texFormat.InternalFormat;
			m_DataFormat = texFormat.DataFormat;

			AR_CORE_ASSERT(m_InternalFormat && m_DataFormat, "Texture", "Formats are not set!");

			glTextureStorage2D(m_TextureID, 4, m_InternalFormat, m_Width, m_Height); // level is number of mipmaps
			glTextureSubImage2D(m_TextureID, 0, 0, 0, m_Width, m_Height, m_DataFormat, GL_UNSIGNED_BYTE, imageData.PixelData);
			// the first 0 is the index of the first level/mipmap

			glGenerateTextureMipmap(m_TextureID);
		}
		else
			AR_CORE_ERROR_TAG("Texture", "Failed to load Texture!{0}", m_Path);

		Utils::ImageLoader::FreeImage();
	}

	void Texture::Bind(uint32_t slot) const
	{
		AR_PROFILE_FUNCTION();

		glBindTextureUnit(slot, m_TextureID);
	}

	void Texture::UnBind(uint32_t slot) const
	{
		AR_PROFILE_FUNCTION();

		glBindTextureUnit(slot, 0);
	}

}