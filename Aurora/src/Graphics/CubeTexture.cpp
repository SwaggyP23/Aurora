#include "Aurorapch.h"
#include "CubeTexture.h"

#include "Utils/ImageLoader.h"

#include <glad/glad.h>
#include <stb_image/stb_image.h>

namespace Aurora {

	CubeTexture::CubeTexture(const std::string& directory)
		: m_Directory(directory)
	{
		LoadFromDirectory();
	}

	CubeTexture::CubeTexture(const std::vector<std::string>& filepaths)
		: m_Directory(), m_Filepaths(filepaths)
	{
		LoadFromFilePaths();
	}

	CubeTexture::~CubeTexture()
	{
		glDeleteTextures(1, &m_TextureID);
	}

	Ref<CubeTexture> CubeTexture::Create(const std::string& filepath)
	{
		return CreateRef<CubeTexture>(filepath);
	}

	Ref<CubeTexture> CubeTexture::Create(const std::vector<std::string>& filepaths)
	{
		return CreateRef<CubeTexture>(filepaths);
	}

	void CubeTexture::Bind(uint32_t slot)
	{
		AR_PROFILE_FUNCTION();

		glBindTextureUnit(slot, m_TextureID);
	}

	void CubeTexture::UnBind(uint32_t slot)
	{
		AR_PROFILE_FUNCTION();

		glBindTextureUnit(slot, 0);
	}

	void CubeTexture::LoadFromDirectory()
	{
		AR_PROFILE_FUNCTION();

		std::vector<std::string> cubeFaces;
		for (auto& path : std::filesystem::directory_iterator(m_Directory))
		{
			cubeFaces.push_back(path.path().string());
		}

		glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &m_TextureID);
		glBindTexture(GL_TEXTURE_CUBE_MAP, m_TextureID);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

		for (int i = 0; i < cubeFaces.size(); i++)
		{
			auto& imageData = Utils::ImageLoader::LoadImageFile(cubeFaces[i]);

			if (imageData.PixelData)
			{
				uint32_t width = imageData.Width;
				uint32_t height = imageData.Height;
				uint32_t numChannels = imageData.Channels;

				GLint internalFormat = 0;
				GLenum format = 0;
				if (numChannels == 1)
				{
					internalFormat = GL_R32I;
					format = GL_RED;
				}
				else if (numChannels == 3)
				{
					internalFormat = GL_RGB8;
					format = GL_RGB;
				}
				else if (numChannels == 4)
				{
					internalFormat = GL_RGBA8;
					format = GL_RGBA;
				}

				glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, internalFormat, width, height, 0, format, GL_UNSIGNED_BYTE, imageData.PixelData);
			}
			else
				AR_CORE_ERROR("Failed to load face: {0}.", cubeFaces[i]);

			Utils::ImageLoader::FreeImage();
		}
	}

	void CubeTexture::LoadFromFilePaths()
	{
		AR_PROFILE_FUNCTION();

		glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &m_TextureID);
		glBindTexture(GL_TEXTURE_CUBE_MAP, m_TextureID);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

		for (int i = 0; i < m_Filepaths.size(); i++)
		{
			auto& imageData = Utils::ImageLoader::LoadImageFile(m_Filepaths[i]);

			if (imageData.PixelData)
			{
				uint32_t width = imageData.Width;
				uint32_t height = imageData.Height;
				uint32_t numChannels = imageData.Channels;

				GLint internalFormat = 0;
				GLenum format = 0;
				if (numChannels == 1)
				{
					internalFormat = GL_R32I;
					format = GL_RED;
				}
				else if (numChannels == 3)
				{
					internalFormat = GL_RGB8;
					format = GL_RGB;
				}
				else if (numChannels == 4)
				{
					internalFormat = GL_RGBA8;
					format = GL_RGBA;
				}

				glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, internalFormat, width, height, 0, format, GL_UNSIGNED_BYTE, imageData.PixelData);
			}
			else
				AR_CORE_ERROR("Failed to load face: {0}.", m_Filepaths[i]);

			Utils::ImageLoader::FreeImage();
		}
	}

}