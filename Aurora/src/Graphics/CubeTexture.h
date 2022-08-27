#pragma once

#include "Core/Base.h"
#include "Texture.h"

#include <filesystem>

namespace Aurora {

	class CubeTexture : public Texture
	{
	public:
		CubeTexture() = default;
		CubeTexture(const std::string& directory); // This constructor is uses the filesystem api to get all the entries
		CubeTexture(const std::vector<std::string>& filepaths);
		virtual ~CubeTexture();

		static Ref<CubeTexture> Create(const std::string& filepath);
		static Ref<CubeTexture> Create(const std::vector<std::string>& filepaths);

		virtual void Bind(uint32_t slot = 0) const override;
		virtual void UnBind(uint32_t slot = 0) const override;

		inline virtual uint32_t GetTextureID() const override { return m_TextureID; }

	private:
		void LoadFromDirectory();
		void LoadFromFilePaths();

	private:
		uint32_t m_TextureID = 0;

		std::filesystem::path m_Directory;
		std::vector<std::string> m_Filepaths;

	};

}