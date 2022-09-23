#include "Aurorapch.h"
#include "Renderer.h"

#include "Core/Application.h"
#include "Renderer3D.h"
#include "RenderCommand.h"

namespace Aurora {

	struct RendererData
	{
		Scope<ShaderLibrary> ShaderLibrary;
		Ref<Texture2D> WhiteTexture;
		Ref<Texture2D> BlackTexture;
		Ref<CubeTexture> BlackCubeTexture;
		Ref<Environment> NullEnvironment;
	};

	static RendererData* s_Data = nullptr;

	// TODO: Maybe should handle UBOs initialization here?
	void Renderer::Init()
	{
		RendererCapabilities::Init();

		s_Data = new RendererData();

		s_Data->ShaderLibrary = ShaderLibrary::Create();

		ShaderProperties renderer2DShaderProps = {};
		renderer2DShaderProps.Name = "MainShader";
		renderer2DShaderProps.AssetPath = "Resources/shaders/MainShader.glsl";
		s_Data->ShaderLibrary->Load(renderer2DShaderProps);

		ShaderProperties skyBoxProps = {};
		skyBoxProps.Name = "Skybox";
		skyBoxProps.AssetPath = "Resources/shaders/Skybox.glsl";
		s_Data->ShaderLibrary->Load(skyBoxProps);

		ShaderProperties auroraPBRStaticProps = {};
		auroraPBRStaticProps.Name = "AuroraPBRStatic";
		auroraPBRStaticProps.AssetPath = "Resources/shaders/AuroraPBRStatic.glsl";
		s_Data->ShaderLibrary->Load(auroraPBRStaticProps);

		ShaderProperties equirectangularToCubeMapProps = {};
		equirectangularToCubeMapProps.Name = "EquirectToCubeMap";
		equirectangularToCubeMapProps.AssetPath = "Resources/shaders/EquiRectangularToCubeMap.glsl";
		equirectangularToCubeMapProps.Type = ShaderType::Compute;
		s_Data->ShaderLibrary->Load(equirectangularToCubeMapProps);

		// TODO: This is a temporary shader that will be dicarded and replaced by the AuroraPBRStatic
		ShaderProperties modelProps = {};
		modelProps.Name = "Model";
		modelProps.AssetPath = "Resources/shaders/model.glsl";
		s_Data->ShaderLibrary->Load(modelProps);

		// 0xABGR
		constexpr uint32_t whiteTextureData = 0xffffffff;
		s_Data->WhiteTexture = Texture2D::Create(ImageFormat::RGBA, 1, 1, &whiteTextureData);

		constexpr uint32_t blackTextureData = 0xff000000;
		s_Data->BlackTexture = Texture2D::Create(ImageFormat::RGBA, 1, 1, &blackTextureData);

		constexpr uint32_t blackCubeTexture[] = { 0xff0000ff, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000 };
		TextureProperties blackCubeTexProps = {};
		blackCubeTexProps.GenerateMips = false;
		s_Data->BlackCubeTexture = CubeTexture::Create(ImageFormat::RGB, 1, 1, (const void*)blackCubeTexture, blackCubeTexProps);

		s_Data->NullEnvironment = Environment::Create(s_Data->BlackCubeTexture, s_Data->BlackCubeTexture);

		RenderCommand::Init();
		Renderer3D::Init();
	}

	void Renderer::ShutDown()
	{
		 Renderer3D::ShutDown();
		 RenderCommand::ShutDown();
		 RendererCapabilities::ShutDown();

		 delete s_Data;
	}

	void Renderer::OnWindowResize(uint32_t width, uint32_t height)
	{
		RenderCommand::SetViewport(0, 0, width, height);
	}

	void Renderer::BeginScene(const Ref<EditorCamera>& camera)
	{
	}

	void Renderer::EndScene()
	{
	}

	const Scope<ShaderLibrary>& Renderer::GetShaderLibrary()
	{
		return s_Data->ShaderLibrary;
	}

	const RendererProperties& Renderer::GetRendererCapabilities()
	{
		return RendererCapabilities::GetRendererProperties();
	}

	const Ref<Texture2D>& Renderer::GetWhiteTexture()
	{
		return s_Data->WhiteTexture;
	}

	const Ref<Texture2D> Renderer::GetBlackTexture()
	{
		return s_Data->BlackTexture;
	}

	const Ref<CubeTexture>& Renderer::GetBlackCubeTexture()
	{
		return s_Data->BlackCubeTexture;
	}

	const Ref<Environment>& Renderer::GetBlackEnvironment()
	{
		return s_Data->NullEnvironment;
	}

}