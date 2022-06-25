#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <Logging/Log.h>

#include <string>

class Texture
{
public:
	Texture(std::string filePath);
	~Texture();

	void setTextureWrapping(GLenum wrapMode) const;
	void setTextureFiltering(GLenum minFilter, GLenum magFilter) const;

	void loadTextureData(GLint internalFormat, GLenum format);

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