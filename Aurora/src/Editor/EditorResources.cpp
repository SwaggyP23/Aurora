#include "Aurorapch.h"
#include "EditorResources.h"

namespace Aurora {

	void EditorResources::Init()
	{
		SearchIcon = LoadTexture("Icons/Search.png");
		GearIcon = LoadTexture("Icons/Gear.png");

		PlayButton = LoadTexture("Viewport/Play.png");
		SimulateButton = LoadTexture("Viewport/Simulate.png");
		PauseButton = LoadTexture("Viewport/Pause.png");
		StopButton = LoadTexture("Viewport/Stop.png");

		TextureProperties cameraProps;
		cameraProps.FlipOnLoad = true;
		CameraIcon = LoadTexture("Icons/Camera.png", cameraProps);

		// Gizmos...
		TranslateIcon = LoadTexture("Viewport/MoveTool.png");
		RotateIcon = LoadTexture("Viewport/RotateTool.png");
		ScaleIcon = LoadTexture("Viewport/ScaleTool.png");
	}

	void EditorResources::Shutdown()
	{
		SearchIcon.Reset();
		GearIcon.Reset();

		PlayButton.Reset();
		SimulateButton.Reset();
		PauseButton.Reset();
		StopButton.Reset();

		// Gizmos...
		TranslateIcon.Reset();
		RotateIcon.Reset();
		ScaleIcon.Reset();

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