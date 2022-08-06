#pragma once

#include "Core/Base.h"

#include <string>

/*
 * With the new OpenGL version 4.5, the texture workflow is no longer Gen -> Bind -> Allocate -> Upload. 
 * Rather now its just like Create -> Allocate -> Upload. And now it is no longer needed to do glActiveTexture(GL_TEXTURE0...);
 * We can just bind to a certain slot which will activate it using glBindTextureUnit(slot, texture);
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

	class Texture
	{
	public:
		Texture(uint32_t width, uint32_t height); // This is to create just default 1x1 RGBA Textures for masking on colors
		Texture(const std::string& filePath);
		~Texture();

		static Ref<Texture> Create(uint32_t width, uint32_t height);
		static Ref<Texture> Create(const std::string& filePath);

		void SetData(const void* data, uint32_t size);

		void SetTextureWrapping(TextureWrap wrapMode) const;
		void SetTextureFiltering(TextureFilter minFilter = TextureFilter::Nearest, TextureFilter magFilter = TextureFilter::Nearest) const;

		// Set to true before calling LoadTextureData() if you want to flip the texture!
		void FlipTextureVertically(bool state);
		void LoadTextureData();

		void Bind(uint32_t slot = 0) const;
		void UnBind(uint32_t slot = 0) const;

		inline uint32_t GetWidth() const { return m_Width; }
		inline uint32_t GetHeight() const { return m_Height; }
		inline uint32_t GetTextureID() const { return m_TextureID; }

		bool operator==(const Texture& other) const { return m_TextureID == other.m_TextureID; }

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