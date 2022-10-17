#pragma once

#include "Texture.h"
#include "Scene/Components.h"

#include <filesystem>

namespace Aurora {
	
	struct MSDFData;

	class Font : public Asset
	{
	public:
		Font(const std::filesystem::path& filepath);
		virtual ~Font();

		static Ref<Font> Create(const std::filesystem::path& filePath);

		Ref<Texture2D> GetFontAtlas() const { return m_TextureAtlas; }
		const MSDFData* GetMSDFData() const { return m_MSDFData; }

		static void Init();
		static void Shutdown();
		static Ref<Font> GetDefaultFont();
		static Ref<Font> GetFontAssetForTextComponent(const TextComponent& textComponent);

		static AssetType GetStaticType() { return AssetType::Font; }
		virtual AssetType GetAssetType() const override { return GetStaticType(); }

	private:
		std::filesystem::path m_FilePath;
		Ref<Texture2D> m_TextureAtlas;
		MSDFData* m_MSDFData = nullptr;

	private:
		static Ref<Font> s_DefaultFont;

	};

}