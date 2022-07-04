#pragma once

#include "OGLpch.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

class Texture
{
public:
	Texture(uint32_t width, uint32_t height); // This is to create just default 1x1 RGBA Textures for masking on colors
	Texture(const std::string& filePath);
	~Texture();

	static Ref<Texture> Create(const std::string& filePath);

	void setData(void* data);

	void setTextureWrapping(GLenum wrapMode) const;
	void setTextureFiltering(GLenum minFilter, GLenum magFilter) const;

	void flipTextureVertically(bool state);
	void loadTextureData();
	// Internal format specifies in what format the texture is to be stored on the GPU
	// format Specifies the format of the pixel data.

	void bind(uint32_t slot = 0) const;
	void unBind() const;

	inline unsigned int GetWidth() const { return m_Width; }
	inline unsigned int GetHeight() const { return m_Height; }

private:
	GLuint m_TextID;
	std::string m_Path;
	uint32_t m_Width, m_Height;

};