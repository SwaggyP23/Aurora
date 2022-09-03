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

		// Components...
		inline static Ref<Texture2D> TransformCompIcon = nullptr;
		inline static Ref<Texture2D> CameraIcon = nullptr;

		static void Init();
		static void Shutdown();

	private:
		static Ref<Texture2D> LoadTexture(const std::filesystem::path& texturePath, const TextureProperties& props = TextureProperties());

	};

}