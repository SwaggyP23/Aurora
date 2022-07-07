#pragma once

#include "OGLpch.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

/*
 * With the new OpenGL version 4.5, the texture workflow is no longer Gen -> Bind -> Allocate -> Upload. 
 * Rather now its just like Create -> Allocate -> Upload. And now it is no longer needed to do glActiveTexture(GL_TEXTURE0...);
 * We can just bind to a certain slot which will activate it using glBindTextureUnit(slot, texture);
 */

class Texture
{
public:
	Texture(uint32_t width, uint32_t height); // This is to create just default 1x1 RGBA Textures for masking on colors
	Texture(const std::string& filePath);
	~Texture();

	static Ref<Texture> Create(uint32_t width, uint32_t height);
	static Ref<Texture> Create(const std::string& filePath);

	void setData(void* data, uint32_t size);

	void setTextureWrapping(GLenum wrapMode = GL_REPEAT) const;
	void setTextureFiltering(GLenum minFilter = GL_NEAREST, GLenum magFilter = GL_NEAREST) const;

	void flipTextureVertically(bool state);
	void loadTextureData();
	// Internal format specifies in what format the texture is to be stored on the GPU
	// format Specifies the format of the pixel data.

	void bind(uint32_t slot = 0) const;
	void unBind(/*uint32_t slot = 0*/) const;

	inline unsigned int GetWidth() const { return m_Width; }
	inline unsigned int GetHeight() const { return m_Height; }

private:
	GLuint m_TextID;
	std::string m_Path;
	uint32_t m_Width, m_Height;
	GLint m_InternalFormat;
	GLenum m_DataFormat;

};