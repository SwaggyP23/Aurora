#include "Aurorapch.h"
#include "EditorResources.h"

namespace Aurora {

	void EditorResources::Init()
	{
		SearchIcon = LoadTexture("Icons/Search.png");
		ClearIcon = LoadTexture("Icons/Clear.png");
		GearIcon = LoadTexture("Icons/Gear.png");
	}

	void EditorResources::Shutdown()
	{
		SearchIcon.Reset();
		ClearIcon.Reset();
		GearIcon.Reset();
	}

	Ref<Texture2D> EditorResources::LoadTexture(const std::filesystem::path& texturePath/*, TextureProperties = TextureProps()*/)
	{
		std::filesystem::path path = std::filesystem::path("Resources") / "EditorInternal" / texturePath;

		if (!std::filesystem::exists(path))
		{
			AR_CORE_CRITICAL_TAG("EditorResources", "Texture Path {} does not exist!", path.string());
			AR_CORE_ASSERT(false);
			return nullptr;
		}

		Ref<Texture2D> texture = Texture2D::Create(path.string());
		texture->SetTextureWrapping(TextureWrap::ClampToEdge);
		texture->SetTextureFiltering(TextureFilter::Linear, TextureFilter::Linear);
		texture->LoadTextureData();

		return texture;
	}

}