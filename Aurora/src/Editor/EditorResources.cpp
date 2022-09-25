#include "Aurorapch.h"
#include "EditorResources.h"

namespace Aurora {

	void EditorResources::Init()
	{
		SearchIcon = LoadTexture("Icons/Search.png");
		ClearIcon = LoadTexture("Icons/Clear.png");
		GearIcon = LoadTexture("Icons/Gear.png");
		CloseIcon = LoadTexture("Icons/Close.png");
		ResetIcon = LoadTexture("Icons/Reset.png");

		//TransformCompIcon = LoadTexture("Icons/transformComponentIcon.png");
		TextureProperties cameraProps;
		cameraProps.FlipOnLoad = true;
		CameraIcon = LoadTexture("Icons/Camera.png", cameraProps);
	}

	void EditorResources::Shutdown()
	{
		SearchIcon.Reset();
		ClearIcon.Reset();
		GearIcon.Reset();
		CloseIcon.Reset();
		ResetIcon.Reset();

		//TransformCompIcon.Reset(); // TODO: Add icons to components...
		CameraIcon.Reset();
	}

	Ref<Texture2D> EditorResources::LoadTexture(const std::string& texturePath, const TextureProperties& props)
	{
		std::filesystem::path path = std::filesystem::path("Resources") / "EditorInternal" / texturePath;

		if (!std::filesystem::exists(path))
		{
			AR_CORE_CRITICAL_TAG("EditorResources", "Texture Path {0} does not exist!", path.string());
			AR_CORE_ASSERT(false);
			return nullptr;
		}

		Ref<Texture2D> texture = Texture2D::Create(path.string(), props);

		return texture;
	}

}