#include "Aurorapch.h"
#include "Texture.h"

#include "Utils/ImageLoader.h"
#include "Renderer/Renderer.h"

#include <glad/glad.h>
#include <stb_image/stb_image.h>

namespace Aurora {

	namespace Utils {

		static GLenum GLFormatFromAFormat(ImageFormat format)
		{
			switch (format)
			{
				case ImageFormat::R8I:						return GL_RED_INTEGER;
				case ImageFormat::R8UI:						return GL_RED_INTEGER;
				case ImageFormat::R16I:						return GL_RED_INTEGER;
				case ImageFormat::R16UI:					return GL_RED_INTEGER;
				case ImageFormat::R32I:						return GL_RED_INTEGER;
				case ImageFormat::R32UI:					return GL_RED_INTEGER;
				case ImageFormat::R32F:						return GL_RED;
				case ImageFormat::RG8:						return GL_RG_INTEGER;
				case ImageFormat::RG16F:					return GL_RG;
				case ImageFormat::RG32F:					return GL_RG;
				case ImageFormat::RGB:						return GL_RGB;
				case ImageFormat::RGBA:						return GL_RGBA;
				case ImageFormat::RGBA16F:					return GL_RGBA;
				case ImageFormat::RGBA32F:					return GL_RGBA;
				case ImageFormat::SRGB:						return GL_RGB;
				case ImageFormat::DEPTH32FSTENCIL8UINT:		return GL_DEPTH_STENCIL;
				case ImageFormat::DEPTH24STENCIL8:			return GL_DEPTH_STENCIL;
				case ImageFormat::DEPTH32F:					return GL_DEPTH_COMPONENT;
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
				case ImageFormat::R32I:						return GL_R32I;
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
				case ImageFormat::DEPTH32FSTENCIL8UINT:		return GL_DEPTH32F_STENCIL8;
				case ImageFormat::DEPTH24STENCIL8:			return GL_DEPTH24_STENCIL8;
				case ImageFormat::DEPTH32F:					return GL_DEPTH_COMPONENT32F;
			}

			AR_CORE_ASSERT(false, "Unknown Image Format!");
			return 0;
		}

		static GLenum GLDataTypeFromAFormat(ImageFormat format)
		{
			switch (format)
			{
				case ImageFormat::R8I:                      return GL_BYTE;
				case ImageFormat::R8UI:						return GL_UNSIGNED_BYTE;
				case ImageFormat::R16I:						return GL_SHORT;
				case ImageFormat::R16UI:					return GL_UNSIGNED_SHORT;
				case ImageFormat::R32I:						return GL_INT;
				case ImageFormat::R32UI:					return GL_UNSIGNED_INT;
				case ImageFormat::R32F:						return GL_FLOAT;
				case ImageFormat::RG8:						return GL_UNSIGNED_BYTE;
				case ImageFormat::RG16F:					return GL_FLOAT;
				case ImageFormat::RG32F:					return GL_FLOAT;
			    case ImageFormat::RGB:						return GL_UNSIGNED_BYTE;
			    case ImageFormat::RGBA:						return GL_UNSIGNED_BYTE;
			    case ImageFormat::RGBA16F:					return GL_FLOAT;
			    case ImageFormat::RGBA32F:					return GL_FLOAT;
			    case ImageFormat::SRGB:						return GL_UNSIGNED_BYTE;
				case ImageFormat::DEPTH32FSTENCIL8UINT:     return GL_FLOAT_32_UNSIGNED_INT_24_8_REV;
			    case ImageFormat::DEPTH24STENCIL8:			return GL_UNSIGNED_INT_24_8;
				case ImageFormat::DEPTH32F:					return GL_FLOAT;
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

	}

	Ref<Texture2D> Texture2D::Create(uint32_t id, uint32_t width, uint32_t height, ImageFormat format, const std::string& path)
	{
		return CreateRef<Texture2D>(id, width, height, format, path);
	}

	Ref<Texture2D> Texture2D::Create(ImageFormat format, uint32_t width, uint32_t height, const void* data, const TextureProperties& props, const std::string& path)
	{
		return CreateRef<Texture2D>(format, width, height, data, props, path);
	}

	Ref<Texture2D> Texture2D::Create(const std::string& filePath, const TextureProperties& props)
	{
		return CreateRef<Texture2D>(filePath, props);
	}

	Texture2D::Texture2D(uint32_t id, uint32_t width, uint32_t height, ImageFormat format, const std::string& path)
		: m_TextureID(id), m_AssetPath(path), m_Width(width), m_Height(height), m_Format(format)
	{
	}

	Texture2D::Texture2D(ImageFormat format, uint32_t width, uint32_t height, const void* data, const TextureProperties& props, const std::string& path)
		: m_AssetPath(path), m_Width(width), m_Height(height), m_Format(format), m_Properties(props)
	{
		AR_PROFILE_FUNCTION();

		m_ImageData = Buffer((Byte*)data, Utils::GetImageMemorySize(format, width, height));

		glCreateTextures(GL_TEXTURE_2D, 1, &m_TextureID);

		Invalidate();
		m_ImageData = Buffer();
	}

	Texture2D::Texture2D(const std::string& filePath, const TextureProperties& props)
		: m_AssetPath(filePath), m_Properties(props), m_Width(0), m_Height(0)
	{
		AR_PROFILE_FUNCTION();

		int32_t width;
		int32_t height;
		int32_t numChannels;

		if (stbi_is_hdr(filePath.c_str()))
		{
			AR_CORE_INFO_TAG("Texture", "Loading an HDR texture from: {0}, SRGB: {1}", filePath.c_str(), props.SRGB);

			stbi_set_flip_vertically_on_load(props.FlipOnLoad);

			float* imageData = stbi_loadf(filePath.c_str(), &width, &height, &numChannels, STBI_rgb_alpha);
			AR_CORE_ASSERT(imageData, "Image was not loaded!");

			stbi_set_flip_vertically_on_load(false); // Reset since stbi is kind of a global state machine!

			m_ImageData = Buffer((Byte*)imageData, Utils::GetImageMemorySize(ImageFormat::RGBA32F, width, height));
			m_Format = ImageFormat::RGBA32F;
		}
		else
		{
			AR_CORE_INFO_TAG("Texture", "Loading a texture from: {0}, SRGB: {1}", filePath.c_str(), props.SRGB);

			stbi_set_flip_vertically_on_load(props.FlipOnLoad);

			Byte* imageData = stbi_load(filePath.c_str(), &width, &height, &numChannels, STBI_rgb_alpha);
			AR_CORE_ASSERT(imageData);

			stbi_set_flip_vertically_on_load(false); // Reset since stbi is kind of a global state machine!

			ImageFormat format = ImageFormat::RGBA;
			m_ImageData = Buffer(imageData, Utils::GetImageMemorySize(format, width, height));
			m_Format = format;
		}

		glCreateTextures(GL_TEXTURE_2D, 1, &m_TextureID);

		// Filtering and wrapping parameters
		glTextureParameteri(m_TextureID, GL_TEXTURE_MIN_FILTER, Utils::GLFilterTypeFromTextureFilter(props.SamplerFilter, props.GenerateMips));
		glTextureParameteri(m_TextureID, GL_TEXTURE_MAG_FILTER, Utils::GLFilterTypeFromTextureFilter(props.SamplerFilter, false));
		glTextureParameteri(m_TextureID, GL_TEXTURE_WRAP_S, Utils::GLWrapTypeFromTextureWrap(props.SamplerWrap));
		glTextureParameteri(m_TextureID, GL_TEXTURE_WRAP_T, Utils::GLWrapTypeFromTextureWrap(props.SamplerWrap));
		glTextureParameteri(m_TextureID, GL_TEXTURE_WRAP_R, Utils::GLWrapTypeFromTextureWrap(props.SamplerWrap));
		glTextureParameterf(m_TextureID, GL_TEXTURE_MAX_ANISOTROPY, glm::min(props.AnisotropicFiltering, Renderer::GetRendererCapabilities().MaxAnisotropy));

		m_Width = width;
		m_Height = height;

		Invalidate();

		stbi_image_free(m_ImageData.Data);
		m_ImageData = Buffer();
	}

	Texture2D::~Texture2D()
	{
		glDeleteTextures(1, &m_TextureID);
		m_TextureID = 0; // Reset textureID just for safety
	}

	void Texture2D::Invalidate()
	{
		AR_PROFILE_FUNCTION();

		GLenum internalFormat = Utils::GLInternalFormatFromAFormat(m_Format);

		uint32_t mipCount = Utils::CalcMipCount(m_Width, m_Height);
		glTextureStorage2D(m_TextureID, mipCount, internalFormat, m_Width, m_Height);
		if (m_ImageData)
		{
			GLenum format = Utils::GLFormatFromAFormat(m_Format);
			GLenum dataType = Utils::GLDataTypeFromAFormat(m_Format);
			glTextureSubImage2D(m_TextureID, 0, 0, 0, m_Width, m_Height, format, dataType, m_ImageData.Data);
			
			if (m_Properties.GenerateMips)
				glGenerateTextureMipmap(m_TextureID);

			m_IsLoaded = true;
		}
	}

	void Texture2D::Bind(uint32_t slot) const
	{
		glBindTextureUnit(slot, m_TextureID);
	}

	void Texture2D::UnBind(uint32_t slot) const
	{
		glBindTextureUnit(slot, 0);
	}

	uint32_t Texture2D::GetMipCount() const
	{
		return Utils::CalcMipCount(m_Width, m_Height);
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	///////// CubeTexture
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	Ref<CubeTexture> CubeTexture::Create(ImageFormat format, uint32_t width, uint32_t height, const void* data, const TextureProperties& props, const std::filesystem::path& filePath)
	{
		return CreateRef<CubeTexture>(format, width, height, data, props, filePath);
	}

	Ref<CubeTexture> CubeTexture::Create(const std::string& filePath, const TextureProperties& props)
	{
		return CreateRef<CubeTexture>(filePath, props);
	}

	CubeTexture::CubeTexture(ImageFormat format, uint32_t width, uint32_t height, const void* data, const TextureProperties& props, const std::filesystem::path& filePath)
		: m_AssetPath(filePath), m_Width(width), m_Height(height), m_Format(format), m_Properties(props)
	{
		// Since we have 6 layers we need memory for 6 * sizeof(one face)
		m_ImageData = Buffer((Byte*)data, Utils::GetImageMemorySize(format, width, height) * 6);

		glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &m_TextureID);
		
		glTextureParameteri(m_TextureID, GL_TEXTURE_MIN_FILTER, Utils::GLFilterTypeFromTextureFilter(props.SamplerFilter, props.GenerateMips));
		glTextureParameteri(m_TextureID, GL_TEXTURE_MAG_FILTER, Utils::GLFilterTypeFromTextureFilter(props.SamplerFilter, false));
		glTextureParameteri(m_TextureID, GL_TEXTURE_WRAP_S, Utils::GLWrapTypeFromTextureWrap(props.SamplerWrap));
		glTextureParameteri(m_TextureID, GL_TEXTURE_WRAP_T, Utils::GLWrapTypeFromTextureWrap(props.SamplerWrap));
		glTextureParameteri(m_TextureID, GL_TEXTURE_WRAP_R, Utils::GLWrapTypeFromTextureWrap(props.SamplerWrap));

		Invalidate();
		m_ImageData = Buffer();
	}

	CubeTexture::CubeTexture(const std::string& filePath, const TextureProperties& props)
		: m_AssetPath(filePath), m_Properties(props)
	{
		AR_CORE_ASSERT(false, "Not Implemented");
	}

	CubeTexture::~CubeTexture()
	{
		glDeleteTextures(1, &m_TextureID);
	}

	void CubeTexture::Invalidate()
	{
		GLenum internalFormat = Utils::GLInternalFormatFromAFormat(m_Format);
		
		uint32_t mipCount = Utils::CalcMipCount(m_Width, m_Height);
		glTextureStorage2D(m_TextureID, mipCount, internalFormat, m_Width, m_Height);
		if (m_ImageData)
		{
			GLenum format = Utils::GLFormatFromAFormat(m_Format);
			GLenum dataType = Utils::GLDataTypeFromAFormat(m_Format);
			glTextureSubImage3D(m_TextureID, 0, 0, 0, 0, m_Width, m_Height, 6, format, dataType, m_ImageData.Data);
		}
	}

	void CubeTexture::Bind(uint32_t slot) const
	{
		glBindTextureUnit(slot, m_TextureID);
	}

	void CubeTexture::UnBind(uint32_t slot) const
	{
		glBindTextureUnit(slot, 0);
	}

	uint32_t CubeTexture::GetMipCount() const
	{
		return Utils::CalcMipCount(m_Width, m_Height);
	}

}