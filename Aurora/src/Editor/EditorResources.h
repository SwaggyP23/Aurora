#pragma once

#include "Core/Base.h"
#include "Graphics/Texture.h"

namespace Aurora {

	class EditorResources
	{
	public:
		inline static Ref<Texture2D> SearchIcon = nullptr;
		inline static Ref<Texture2D> ClearIcon = nullptr;
		inline static Ref<Texture2D> GearIcon = nullptr;
		inline static Ref<Texture2D> CloseIcon = nullptr;
		inline static Ref<Texture2D> ResetIcon = nullptr;

		inline static Ref<Texture2D> PlayButton = nullptr;
		inline static Ref<Texture2D> SimulateButton = nullptr;
		inline static Ref<Texture2D> PauseButton = nullptr;
		inline static Ref<Texture2D> StopButton = nullptr;

		// Gizmos...
		inline static Ref<Texture2D> TranslateIcon = nullptr;
		inline static Ref<Texture2D> RotateIcon = nullptr;
		inline static Ref<Texture2D> ScaleIcon = nullptr;

		// Icons...
		inline static Ref<Texture2D> CameraIcon = nullptr;

		static void Init();
		static void Shutdown();

	private:
		static Ref<Texture2D> LoadTexture(const std::string& texturePath, const TextureProperties& props = TextureProperties());

	};

}