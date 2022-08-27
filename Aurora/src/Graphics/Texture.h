#pragma once

#include "Core/Base.h"

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

	enum class TextureWrap // Wrapping settings
	{
		None = 0,
		Repeat,
		MirrorredRepeat, 
		ClampToEdge, 
		ClampToBorder
	};

	enum class TextureFilter // Filtering settings
	{
		None = 0, 
		Nearest,
		Linear,
		MipMap_NearestNearest,
		MipMap_LinearNearest,
		MipMap_NearestLinear,
		MipMap_LinearLinear
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
		Texture2D(uint32_t width, uint32_t height); // This is to create just default 1x1 RGBA Textures for masking on colors
		Texture2D(const std::string& filePath);
		virtual ~Texture2D();

		static Ref<Texture2D> Create(uint32_t width, uint32_t height);
		static Ref<Texture2D> Create(const std::string& filePath);

		void SetData(const void* data, uint32_t size);

		void SetTextureWrapping(TextureWrap wrapMode) const;
		void SetTextureFiltering(TextureFilter minFilter = TextureFilter::Nearest, TextureFilter magFilter = TextureFilter::Nearest) const;

		// Set to true before calling LoadTextureData() if you want to flip the texture!
		void FlipTextureVertically(bool state);
		void LoadTextureData();

		virtual void Bind(uint32_t slot = 0) const override;
		virtual void UnBind(uint32_t slot = 0) const override;

		inline uint32_t GetWidth() const { return m_Width; }
		inline uint32_t GetHeight() const { return m_Height; }
		inline virtual uint32_t GetTextureID() const override { return m_TextureID; }

		bool operator==(const Texture2D& other) const { return m_TextureID == other.m_TextureID; }

	private:
		uint32_t m_TextureID = 0;
		std::string m_Path;

		uint32_t m_Width, m_Height;

		// Internal format specifies in what format the texture is to be stored on the GPU
        // format Specifies the format of the pixel data.
		int m_InternalFormat;
		uint32_t/*GLenum*/ m_DataFormat;

	};

}