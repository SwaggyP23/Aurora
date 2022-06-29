#pragma once

#include "OGLpch.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

class Texture
{
public:
	Texture(std::string filePath);
	~Texture();

	void setTextureWrapping(GLenum wrapMode) const;
	void setTextureFiltering(GLenum minFilter, GLenum magFilter) const;

	void loadTextureData(GLint internalFormat, GLenum format);
	// Internal format specifies in what format the texture is to be stored on the GPU
	// format Specifies the format of the pixel data.

	void bind() const;
	void unBind() const;

	inline unsigned int GetWidth() const { return m_Width; }
	inline unsigned int GetHeight() const { return m_Height; }
	//inline GLuint getTextureID() const { return m_TextID; }

private:
	GLuint m_TextID;
	std::string m_Path;
	uint32_t m_Width, m_Height;

};