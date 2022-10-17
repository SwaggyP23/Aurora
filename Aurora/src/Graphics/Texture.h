#pragma once

#include "AssetManager/Asset.h"
#include "Core/Base.h"
#include "Core/Buffer.h"

#include <string>

/*
 * With the new OpenGL version 4.5, the texture workflow is no longer Gen -> Bind -> Allocate -> Upload. 
 * Rather now its just like Create -> Allocate -> Upload. And now it is no longer needed to do glActiveTexture(GL_TEXTURE0...);
 * We can just bind to a certain slot which will activate it using glBindTextureUnit(slot, texture);
 * 
 * Fow now, once the texture is loaded the image/pixel data is not held on localy, it is released directly after
 * invalidation.
 */

namespace Aurora {

	enum class ImageFormat : uint8_t
	{
		None = 0,

		// Color
		R8I,
		R8UI,
		R16I,
		R16UI,
		R32I,
		R32UI,
		R32F,
		RG8,
		RG16F,
		RG32F,
		RGB,
		RGBA,
		RGBA16F,
		RGBA32F,

		SRGB, // Currently not supported

		// Depth / Stencil
		DEPTH24STENCIL8,
		DEPTH32FSTENCIL8UINT,
		DEPTH32F,

		// Defaults
		Depth = DEPTH24STENCIL8
	};

	enum class ImageUsage : uint8_t
	{
		None = 0,
		Texture,
		Attachment,
		Storage
	};

	enum class TextureWrap : uint8_t // Wrapping settings
	{
		None = 0,
		Repeat,
		Clamp
	};

	enum class TextureFilter : uint8_t // Filtering settings
	{
		None = 0, 
		Nearest,
		Linear,
		Cubic
	};

	struct TextureProperties
	{
		std::string DebugName;
		TextureWrap SamplerWrap = TextureWrap::Repeat;
		TextureFilter SamplerFilter = TextureFilter::Linear;
		float AnisotropicFiltering = 1.0f; // For anisotropic filtering

		bool FlipOnLoad = false;
		bool GenerateMips = false;
		// Currently not in use! However it is used to determine the number of channels to be loaded with stb
		bool SRGB = false;
	};

	class Texture : public Asset
	{
	public:
		virtual ~Texture() = default;

		virtual void Invalidate() = 0;

		virtual void Bind(uint32_t slot = 0) const = 0;
		virtual void UnBind(uint32_t slot = 0) const = 0;

		virtual uint32_t GetTextureID() const = 0;
		virtual uint32_t GetWidth() const = 0;
		virtual uint32_t GetHeight() const = 0;
		virtual uint32_t GetMipCount() const = 0;
		virtual const std::filesystem::path& GetAssetPath() const = 0;
		virtual ImageFormat GetFormat() const = 0;
		virtual bool IsLoaded() const = 0;

	};

	class Texture2D : public Texture
	{
	public:
		Texture2D(uint32_t id, uint32_t width, uint32_t height, ImageFormat format, const std::string& path = "");
		Texture2D(const std::string& filePath, const TextureProperties& props = TextureProperties());
		Texture2D(ImageFormat format, uint32_t width, uint32_t height, const void* data = nullptr, const TextureProperties& props = TextureProperties(), const std::string& path = "");
		virtual ~Texture2D();

		[[nodiscard]] static Ref<Texture2D> Create(uint32_t id, uint32_t width, uint32_t height, ImageFormat format, const std::string& path = "");
		[[nodiscard]] static Ref<Texture2D> Create(const std::string& filePath, const TextureProperties& props = TextureProperties());
		[[nodiscard]] static Ref<Texture2D> Create(ImageFormat format, uint32_t width, uint32_t height, const void* data = nullptr, const TextureProperties& props = TextureProperties(), const std::string& path = "");

		virtual void Bind(uint32_t slot = 0) const override;
		virtual void UnBind(uint32_t slot = 0) const override;

		[[nodiscard]] inline virtual uint32_t GetTextureID() const override { return m_TextureID; }
		[[nodiscard]] inline virtual uint32_t GetWidth() const override { return m_Width; }
		[[nodiscard]] inline virtual uint32_t GetHeight() const override { return m_Height; }
		[[nodiscard]] virtual uint32_t GetMipCount() const override;
		[[nodiscard]] inline virtual const std::filesystem::path& GetAssetPath() const override { return m_AssetPath; }
		[[nodiscard]] inline virtual ImageFormat GetFormat() const override { return m_Format; }
		[[nodiscard]] inline virtual bool IsLoaded() const override { return m_IsLoaded; }
		[[nodiscard]] inline TextureProperties& GetTextureProperties() { return m_Properties; }

		bool operator==(const Ref<Texture2D>& other) const { return m_TextureID == other->m_TextureID; }

		static AssetType GetStaticType() { return AssetType::Texture; }
		virtual AssetType GetAssetType() const override { return GetStaticType(); }

	private:
		virtual void Invalidate() override;

	private:
		uint32_t m_TextureID = 0;
		std::filesystem::path m_AssetPath;
		TextureProperties m_Properties;

		uint32_t m_Width = 0;
		uint32_t m_Height = 0;
		Buffer m_ImageData = nullptr;

		bool m_IsLoaded = false;

		ImageFormat m_Format = ImageFormat::None;

	};

	class CubeTexture : public Texture
	{
	public:
		CubeTexture(ImageFormat format, uint32_t width, uint32_t height, const void* data = nullptr, const TextureProperties& props = TextureProperties(), const std::filesystem::path& filePath = "");
		CubeTexture(const std::string& filePath, const TextureProperties& props = TextureProperties());
		virtual ~CubeTexture();

		static Ref<CubeTexture> Create(ImageFormat format, uint32_t width, uint32_t height, const void* data = nullptr, const TextureProperties& props = TextureProperties(), const std::filesystem::path& filePath = "");
		static Ref<CubeTexture> Create(const std::string& filePath, const TextureProperties& props = TextureProperties());

		virtual void Bind(uint32_t slot = 0) const override;
		virtual void UnBind(uint32_t slot = 0) const override;

		[[nodiscard]] virtual uint32_t GetTextureID() const override { return m_TextureID; }
		[[nodiscard]] virtual uint32_t GetWidth() const override { return m_Width; }
		[[nodiscard]] virtual uint32_t GetHeight() const override { return m_Height; }
		[[nodiscard]] virtual uint32_t GetMipCount() const override;
		[[nodiscard]] virtual const std::filesystem::path& GetAssetPath() const override { return m_AssetPath; }
		[[nodiscard]] virtual ImageFormat GetFormat() const override { return m_Format; }
		[[nodiscard]] virtual bool IsLoaded() const override { return m_IsLoaded; }

		bool operator==(const Ref<CubeTexture>& other) const { return m_TextureID == other->m_TextureID; }

		static AssetType GetStaticType() { return AssetType::EnvironmentMap; }
		virtual AssetType GetAssetType() const override { return GetStaticType(); }

	private:
		virtual void Invalidate() override;

	private:
		uint32_t m_TextureID = 0;
		TextureProperties m_Properties;
		ImageFormat m_Format = ImageFormat::None;
		std::filesystem::path m_AssetPath;
		
		uint32_t m_Width = 0;
		uint32_t m_Height = 0;
		Buffer m_ImageData = nullptr;

		bool m_IsLoaded = false;

	};

	namespace Utils {

		static uint32_t GetImageFormatBPP(ImageFormat format)
		{
			switch (format)
			{
			    case ImageFormat::R8UI:        return 1;
			    case ImageFormat::R16UI:       return 2;
			    case ImageFormat::R32UI:       return 4;
			    case ImageFormat::R32F:        return 4;
				case ImageFormat::RG16F:	   return 2 * 4;
			    case ImageFormat::RGB:	       return 3;
			    case ImageFormat::SRGB:        return 3;
			    case ImageFormat::RGBA:        return 4;
			    case ImageFormat::RGBA16F:     return 2 * 4;
			    case ImageFormat::RGBA32F:     return 4 * 4;
			}

			AR_CORE_ASSERT(false, "Unknown Image Format!");
			return 0;
		}

		static uint32_t GetImageMemorySize(ImageFormat format, uint32_t width, uint32_t height)
		{
			return width * height * GetImageFormatBPP(format);
		}

		static uint32_t CalcMipCount(uint32_t width, uint32_t height)
		{
			return (uint32_t)std::floor(std::log2(std::min(width, height))) + 1;
		}

	}

}