#pragma once

#include "Core/Base.h"

#include <filesystem>

namespace Aurora {

	class CubeTexture
	{
	public:
		CubeTexture() = default;
		CubeTexture(const std::string& directory); // This constructor is uses the filesystem api to get all the entries
		CubeTexture(const std::vector<std::string>& filepaths);
		~CubeTexture();

		static Ref<CubeTexture> Create(const std::string& filepath);
		static Ref<CubeTexture> Create(const std::vector<std::string>& filepaths);

		void Bind(uint32_t slot = 0);
		void UnBind(uint32_t slot = 0);

		inline uint32_t GetTextureID() const { return m_TextureID; }

	private:
		void LoadFromDirectory();
		void LoadFromFilePaths();

	private:
		uint32_t m_TextureID = 0;

		std::filesystem::path m_Directory;
		std::vector<std::string> m_Filepaths;

	};

}