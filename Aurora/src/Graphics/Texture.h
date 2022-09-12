#pragma once

#include "Core/Base.h"
#include "Core/Buffer.h"

#include <string>

/*
 * With the new OpenGL version 4.5, the texture workflow is no longer Gen -> Bind -> Allocate -> Upload. 
 * Rather now its just like Create -> Allocate -> Upload. And now it is no longer needed to do glActiveTexture(GL_TEXTURE0...);
 * We can just bind to a certain slot which will activate it using glBindTextureUnit(slot, texture);
 * 
 * TODO: Create a texture properties struct so that we can create textures by providing properties just like the framebuffers
 * workflow.
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
		Storage // Currently not used for anything
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
	};

	struct TextureProperties
	{
		std::string DebugName;
		TextureWrap SamplerWrap = TextureWrap::Repeat;
		TextureFilter SamplerFilter = TextureFilter::Linear;
		bool FlipOnLoad = false;
		bool GenerateMips = true;
		bool SRGB = false; // Currently not supported! However it is used to determine the number of channels to be loaded with stb
	};

	class Texture : public RefCountedObject
	{
	public:
		virtual ~Texture() = default;

		virtual void Bind(uint32_t slot = 0) const = 0;
		virtual void UnBind(uint32_t slot = 0) const = 0;

		virtual uint32_t GetTextureID() const = 0;

	};

	class Texture2D : public Texture
	{
	public:
		Texture2D(const std::string& filePath, const TextureProperties& props = TextureProperties());
		Texture2D(ImageFormat format, uint32_t width, uint32_t height, const void* data, const TextureProperties& props = TextureProperties());
		virtual ~Texture2D();

		static Ref<Texture2D> Create(const std::string& filePath, const TextureProperties& props = TextureProperties());
		static Ref<Texture2D> Create(ImageFormat format, uint32_t width, uint32_t height, const void* data, const TextureProperties& props = TextureProperties());

		void Invalidate();

		virtual void Bind(uint32_t slot = 0) const override;
		virtual void UnBind(uint32_t slot = 0) const override;

		[[nodiscard]] inline uint32_t GetWidth() const { return m_Width; }
		[[nodiscard]] inline uint32_t GetHeight() const { return m_Height; }
		[[nodiscard]] inline const std::string& GetAssetPath() const { return m_AssetPath; }
		[[nodiscard]] inline virtual uint32_t GetTextureID() const override { return m_TextureID; }
		[[nodiscard]] inline TextureProperties& GetTextureProperties() { return m_Properties; }

		bool operator==(const Texture2D& other) const { return m_TextureID == other.m_TextureID; }

	private:
		uint32_t m_TextureID = 0;
		std::string m_AssetPath;
		TextureProperties m_Properties;

		uint32_t m_Width;
		uint32_t m_Height;
		Buffer m_ImageData;

		ImageFormat m_Format = ImageFormat::None;

	};

}