#pragma once

#include "Core/Base.h"
#include "RendererCaps.h"
#include "SceneRenderer.h"
#include "Graphics/Shader.h"
#include "Graphics/RenderPass.h"
#include "Graphics/Texture.h"
#include "Graphics/UniformBuffer.h"
#include "Graphics/StorageBuffer.h"
#include "Graphics/Material.h"
#include "Graphics/Mesh.h"
#include "Graphics/SceneEnvironment.h"
#include "Editor/EditorCamera.h"

namespace Aurora {

	struct RendererConfig
	{
		// Tiering settings
		uint32_t EnvironmentMapResolution = 1024;
		uint32_t IrradianceMapComputeSamples = 1024;
	};

	class Renderer
	{
	public:
		static void Init(const RendererConfig& configuration = RendererConfig());
		static void ShutDown();

		static RendererConfig& GetConfig();

		[[nodiscard]] static const RendererProperties& GetRendererCapabilities();
		[[nodiscard]] static const Scope<ShaderLibrary>& GetShaderLibrary();

		[[nodiscard]] static const Ref<Texture2D>& GetWhiteTexture();
		[[nodiscard]] static const Ref<Texture2D>& GetBlackTexture();
		[[nodiscard]] static const Ref<Texture2D>& GetBRDFLutTexture();
		[[nodiscard]] static const Ref<CubeTexture>& GetBlackCubeTexture();
		[[nodiscard]] static const Ref<Environment>& GetBlackEnvironment();

		static void SetSceneEnvironment(Ref<SceneRenderer> sceneRenderer, Ref<Environment> environment);
		static std::pair<Ref<CubeTexture>, Ref<CubeTexture>> CreateEnvironmentMap(const std::string& filePath);
		static Ref<CubeTexture> CreatePreethamSky(float turbidity, float azimuth, float inclination);

		// TODO: Maybe have an s_Data struct and set all the uniform buffer and stuff like that there? temp untill
		// scenerenderer is a thing
		static void BeginScene(const Ref<EditorCamera>& camera);
		static void EndScene();

		static void BeginRenderPass(const Ref<RenderPass>& renderPass);
		static void EndRenderPass(const Ref<RenderPass>& renderPass);

		static void SubmitFullScreenQuad(const Ref<Material>& material);
		// TODO: Work on rendering static meshes. NOTE: Should go through all the submeshes and draw them also
		static void RenderStaticMesh(const Ref<StaticMesh>& mesh, const Ref<MaterialTable>& material, const glm::mat4& tranform);
		static void RenderGeometry(const Ref<UniformBuffer>& ubo, const Ref<StorageBuffer>& ssbo, const Ref<Material>& mat, const Ref<VertexArray>& vao, uint32_t indexCount = 0);
		static void RenderQuad(const Ref<Material>& material, const glm::mat4& transform);
		
		static void OnWindowResize(uint32_t width, uint32_t height);

	private:
		static void GenOrGetBRDFLutTexture();

	};

}