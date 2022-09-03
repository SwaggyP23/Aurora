#include "Aurorapch.h"
#include "Texture.h"

#include "Utils/ImageLoader.h"

#include <glad/glad.h>
#include <stb_image/stb_image.h>

namespace Aurora {

	namespace Utils {

		static GLenum GLFormatFromAFormat(ImageFormat format)
		{
			switch (format)
			{
			    case Aurora::ImageFormat::RGB:		return GL_RGB;
			    case Aurora::ImageFormat::RGBA:		return GL_RGBA;
			    case Aurora::ImageFormat::RGBA16F:  return GL_RGBA;
			    case Aurora::ImageFormat::RGBA32F:  return GL_RGBA;
			    case Aurora::ImageFormat::SRGB:		return GL_RGB;
			}
			
			AR_CORE_ASSERT(false, "Unknown Image Format!");
			return 0;
		}

		static GLenum GLInternalFormatFromAFormat(ImageFormat format)
		{
			switch (format)
			{
			    case ImageFormat::None:						return GL_NONE;
			    case ImageFormat::R8I:						return GL_R8I;
			    case ImageFormat::R8UI:						return GL_R8UI;
			    case ImageFormat::R16I:						return GL_R16I;
			    case ImageFormat::R16UI:					return GL_R16UI;
			    case ImageFormat::R32UI:					return GL_R32UI;
			    case ImageFormat::R32F:						return GL_R32F;
			    case ImageFormat::RG8:						return GL_RG8;
			    case ImageFormat::RG16F:					return GL_RG16F;
			    case ImageFormat::RG32F:					return GL_RG32F;
			    case ImageFormat::RGB:						return GL_RGB8;
			    case ImageFormat::RGBA:						return GL_RGBA8;
			    case ImageFormat::RGBA16F:					return GL_RGBA16F;
			    case ImageFormat::RGBA32F:					return GL_RGBA32F;
				case ImageFormat::SRGB:						return GL_SRGB8;
			    case ImageFormat::DEPTH24STENCIL8:			return GL_DEPTH24_STENCIL8;
			    case ImageFormat::DEPTH32FSTENCIL8UINT:		return GL_DEPTH32F_STENCIL8;
			    case ImageFormat::DEPTH32F:					return GL_DEPTH_COMPONENT32F;
			}

			AR_CORE_ASSERT(false, "Unknown Image Format!");
			return 0;
		}

		static GLenum GLDataTypeFromAFormat(ImageFormat format)
		{
			switch (format)
			{
			    case Aurora::ImageFormat::RGB:		return GL_UNSIGNED_BYTE;
			    case Aurora::ImageFormat::RGBA:		return GL_UNSIGNED_BYTE;
			    case Aurora::ImageFormat::RGBA16F:  return GL_FLOAT;
			    case Aurora::ImageFormat::RGBA32F:  return GL_FLOAT;
			    case Aurora::ImageFormat::SRGB:		return GL_UNSIGNED_BYTE;
			}

			AR_CORE_ASSERT(false, "Unknown Image Format!");
			return 0;
		}

		static GLenum GLFilterTypeFromTextureFilter(TextureFilter type, bool hasMipmap)
		{
			if (type == TextureFilter::None)                        return GL_NONE;
			else if (type == TextureFilter::Nearest && hasMipmap)   return GL_NEAREST_MIPMAP_NEAREST;
			else if (type == TextureFilter::Linear && hasMipmap)    return GL_LINEAR_MIPMAP_LINEAR;
			else if (type == TextureFilter::Nearest && !hasMipmap)  return GL_NEAREST;
			else if (type == TextureFilter::Linear && !hasMipmap)   return GL_LINEAR;

			AR_CORE_ASSERT(false, "Unknown Texture Filter!");
			return 0;
		}

		static GLenum GLWrapTypeFromTextureWrap(TextureWrap type)
		{
			switch (type)
			{
			    case TextureWrap::None:               return GL_NONE;
			    case TextureWrap::Repeat:             return GL_REPEAT;
			    case TextureWrap::Clamp:              return GL_CLAMP_TO_EDGE;
			}

			AR_CORE_ASSERT(false, "Unknown Texture Wrap Mode!");
			return 0;
		}

		// TODO: Move into a header file since these will be used commonly
		static uint32_t GetImageFormatBPP(ImageFormat format)
		{
			switch (format)
			{
			    case ImageFormat::R8UI:      return 1;
			    case ImageFormat::R16UI:     return 2;
			    case ImageFormat::R32UI:     return 4;
			    case ImageFormat::R32F:      return 4;
			    case ImageFormat::RGB:	       
			    //case ImageFormat::SRGB:        return 3;
			    case ImageFormat::RGBA:        return 4;
			    case ImageFormat::RGBA16F:     return 2 * 4;
			    case ImageFormat::RGBA32F:     return 4 * 4;
			}

			AR_CORE_ASSERT(false, "Unknown Image Format!");
			return 0;
		}

		static uint32_t GetImageMemorySize(ImageFormat format, int width, int height)
		{
			return width * height * GetImageFormatBPP(format);
		}

		static uint32_t CalcMipCount(uint32_t width, uint32_t height)
		{
			return (uint32_t)std::floor(std::log2(std::min(width, height))) + 1;
		}

	}

	Ref<Texture2D> Texture2D::Create(ImageFormat format, uint32_t width, uint32_t height, const void* data, const TextureProperties& props)
	{
		return CreateRef<Texture2D>(format, width, height, data, props);
	}

	Ref<Texture2D> Texture2D::Create(const std::string& filePath, const TextureProperties& props)
	{
		return CreateRef<Texture2D>(filePath, props);
	}

	Texture2D::Texture2D(ImageFormat format, uint32_t width, uint32_t height, const void* data, const TextureProperties& props)
		: m_Width(width), m_Height(height), m_Format(format), m_Properties(props)
	{
		AR_PROFILE_FUNCTION();
		m_ImageData = Buffer((void*)data, Utils::GetImageMemorySize(format, width, height));

		glCreateTextures(GL_TEXTURE_2D, 1, &m_TextureID);

		Invalidate();
		m_ImageData = Buffer(); // Release local memory buffer after it is created on the gpu
	}

	Texture2D::Texture2D(const std::string& filePath, const TextureProperties& props)
		: m_AssetPath(filePath), m_Properties(props), m_Width(0), m_Height(0)
	{
		AR_PROFILE_FUNCTION();

		int32_t width, height, numChannels;

		if (stbi_is_hdr(filePath.c_str()))
		{
			// TODO: Log if the texture is SRGB or not!
			AR_CORE_INFO_TAG("Texture", "Loading an HDR texture from: {0}, SRGB: {1}", filePath.c_str(), props.SRGB);

			stbi_set_flip_vertically_on_load(props.FlipOnLoad);

			float* imageData = stbi_loadf(filePath.c_str(), &width, &height, &numChannels, STBI_rgb_alpha);
			AR_CORE_ASSERT(imageData, "Image was not loaded!");

			stbi_set_flip_vertically_on_load(false); // Reset since it is kind of a global state inside stb!

			m_ImageData = Buffer(imageData, Utils::GetImageMemorySize(ImageFormat::RGBA32F, width, height));
			m_Format = ImageFormat::RGBA32F;
		}
		else
		{
			AR_CORE_INFO_TAG("Texture", "Loading a texture from: {0}, SRGB: {1}", filePath.c_str(), props.SRGB);

			stbi_set_flip_vertically_on_load(props.FlipOnLoad);

			uint8_t* imageData = stbi_load(filePath.c_str(), &width, &height, &numChannels, props.SRGB ? STBI_rgb : STBI_rgb_alpha);
			AR_CORE_ASSERT(imageData);

			stbi_set_flip_vertically_on_load(false); // Reset since it is kind of a global state inside stb!

			ImageFormat format = props.SRGB ? ImageFormat::RGB : ImageFormat::RGBA;
			m_ImageData = Buffer(imageData, Utils::GetImageMemorySize(format, width, height));
			m_Format = format;
		}

		glCreateTextures(GL_TEXTURE_2D, 1, &m_TextureID);

		glTextureParameteri(m_TextureID, GL_TEXTURE_MIN_FILTER, Utils::GLFilterTypeFromTextureFilter(props.SamplerFilter, props.GenerateMips));
		glTextureParameteri(m_TextureID, GL_TEXTURE_MAG_FILTER, Utils::GLFilterTypeFromTextureFilter(props.SamplerFilter, false));
		glTextureParameteri(m_TextureID, GL_TEXTURE_WRAP_R, Utils::GLWrapTypeFromTextureWrap(props.SamplerWrap));
		glTextureParameteri(m_TextureID, GL_TEXTURE_WRAP_S, Utils::GLWrapTypeFromTextureWrap(props.SamplerWrap));
		glTextureParameteri(m_TextureID, GL_TEXTURE_WRAP_T, Utils::GLWrapTypeFromTextureWrap(props.SamplerWrap));

		m_Width = width;
		m_Height = height;

		Invalidate();

		stbi_image_free(m_ImageData.GetData());
		m_ImageData = Buffer(); // Reset the buffer, TODO: Check if this whole constructor actually behaves as it should
		// check if the data being freed here above is the actual data that should be freed! And also check the data
		// loading process if its legit, whole constructor looks sus!
	}

	Texture2D::~Texture2D()
	{
		AR_PROFILE_FUNCTION();

		glDeleteTextures(1, &m_TextureID);
		m_TextureID = 0; // Reset textureID just for safety
	}

	void Texture2D::Invalidate()
	{
		AR_PROFILE_FUNCTION();

		GLenum format, internalFormat;
		internalFormat = Utils::GLInternalFormatFromAFormat(m_Format);

		uint32_t mipCount = Utils::CalcMipCount(m_Width, m_Height);
		glTextureStorage2D(m_TextureID, mipCount, internalFormat, m_Width, m_Height);
		if (m_ImageData)
		{
			format = Utils::GLFormatFromAFormat(m_Format);
			GLenum dataType = Utils::GLDataTypeFromAFormat(m_Format);
			glTextureSubImage2D(m_TextureID, 0, 0, 0, m_Width, m_Height, format, dataType, m_ImageData.GetData());
			
			if (m_Properties.GenerateMips)
				glGenerateTextureMipmap(m_TextureID);
		}
	}

	void Texture2D::Bind(uint32_t slot) const
	{
		AR_PROFILE_FUNCTION();

		glBindTextureUnit(slot, m_TextureID);
	}

	void Texture2D::UnBind(uint32_t slot) const
	{
		AR_PROFILE_FUNCTION();

		glBindTextureUnit(slot, 0);
	}

}