#pragma once

#include "Core/Base.h"
#include "RendererCaps.h"
#include "Graphics/Shader.h"
#include "Graphics/Texture.h"
#include "Graphics/SceneEnvironment.h"
#include "Editor/EditorCamera.h"

namespace Aurora {

	class Renderer
	{
	public:
		static void Init();
		static void ShutDown();

		static void OnWindowResize(uint32_t width, uint32_t height);

		[[nodiscard]] static const RendererProperties& GetRendererCapabilities();
		[[nodiscard]] static const Scope<ShaderLibrary>& GetShaderLibrary();

		[[nodiscard]] static const Ref<Texture2D>& GetWhiteTexture();
		[[nodiscard]] static const Ref<Texture2D> GetBlackTexture();
		[[nodiscard]] static const Ref<CubeTexture>& GetBlackCubeTexture();
		[[nodiscard]] static const Ref<Environment>& GetBlackEnvironment();

		// TODO: Maybe have an s_Data struct and set all the uniform buffer and stuff like that there? temp untill
		// scenerenderer is a thing
		static void BeginScene(const Ref<EditorCamera>& camera);
		static void EndScene();

		//static void DrawSkyBox(const Ref<CubeTexture>& skybox);
		//static void DrawMaterial(const Ref<Material>& mat, const glm::mat4& trans, const glm::vec4& tint);

		// TODO: Should go through all the submeshes and draw them also
		// static void RenderStaticMesh(mesh.. material... vertexbuffer... whatever);

	};

}