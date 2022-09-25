#include "Aurorapch.h"
#include "Renderer.h"

#include "Core/Application.h"
#include "Renderer3D.h"
#include "RenderCommand.h"
#include "Utils/ImageLoader.h"

// Need this for the environment map
#include <glad/glad.h>

namespace Aurora {

	struct RendererData
	{
		Scope<ShaderLibrary> ShaderLibrary;
		Ref<Texture2D> WhiteTexture;
		Ref<Texture2D> BlackTexture;
		Ref<Texture2D> BRDFLutTexture;
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

		ShaderProperties environmentLevelFilter = {};
		environmentLevelFilter.Name = "EnvironmentMipFilter";
		environmentLevelFilter.AssetPath = "Resources/shaders/EnvironmentLevelFilter.glsl";
		environmentLevelFilter.Type = ShaderType::Compute;
		s_Data->ShaderLibrary->Load(environmentLevelFilter);

		ShaderProperties environmentIrradiance = {};
		environmentIrradiance.Name = "EnvironmentIrradiance";
		environmentIrradiance.AssetPath = "Resources/shaders/EnvironmentIrradiance.glsl";
		environmentIrradiance.Type = ShaderType::Compute;
		s_Data->ShaderLibrary->Load(environmentIrradiance);

		ShaderProperties BRDFLutShaderProps = {};
		BRDFLutShaderProps.Name = "BRDFLutGen";
		BRDFLutShaderProps.AssetPath = "Resources/shaders/BRDFLutGen.glsl";
		BRDFLutShaderProps.Type = ShaderType::Compute;
		s_Data->ShaderLibrary->Load(BRDFLutShaderProps);

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

		constexpr uint32_t blackCubeTexture[] = { 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000 };
		TextureProperties blackCubeTexProps = {};
		blackCubeTexProps.GenerateMips = false;
		s_Data->BlackCubeTexture = CubeTexture::Create(ImageFormat::RGB, 1, 1, (const void*)blackCubeTexture, blackCubeTexProps);

		s_Data->NullEnvironment = Environment::Create(s_Data->BlackCubeTexture, s_Data->BlackCubeTexture);

		GenOrGetBRDFLutTexture();

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

	std::pair<Ref<CubeTexture>, Ref<CubeTexture>> Renderer::CreateEnvironmentMap(const std::string& filePath)
	{
		const uint32_t cubeMapSize = 1024; // TODO: This should be a rendererConfig global setting and could be set from editor
		constexpr uint32_t irradianceMapSize = 32;
		const uint32_t irradianceMapSamples = 1024; // TODO: This should be a rendererConfig global setting and could be set from editor

		// This is to properly apply mipmap linear filtering to the cubeTexture that require it: envUnfiltered, envFiltered, irradianceMap
		TextureProperties cubeTextureProps = {};
		cubeTextureProps.GenerateMips = true;

		Ref<Shader> equiRectToCubeShader = s_Data->ShaderLibrary->TryGet("EquirectToCubeMap");
		Ref<CubeTexture> envUnfiltered = CubeTexture::Create(ImageFormat::RGBA32F, cubeMapSize, cubeMapSize, nullptr, cubeTextureProps);
		Ref<Texture2D> envEquiRect = Texture2D::Create(filePath);
		AR_CORE_ASSERT(envEquiRect->GetFormat() == ImageFormat::RGBA32F, "Texture should be HDR!");

		// Dispatch compute shader to change from Equirect to CubeMap
		{
			AR_SCOPED_TIMER("LatLong to CubeMap stage");
			AR_PROFILE_SCOPE("LatLong to CubeMap stage");

			equiRectToCubeShader->Bind();
			envEquiRect->Bind(0);

			glBindImageTexture(0, envUnfiltered->GetTextureID(), 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA32F);
			equiRectToCubeShader->Dispatch(cubeMapSize / 32, cubeMapSize / 32, 6);
			// Make sure the image is completely written to before accessing it again
			glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT | GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
			glGenerateTextureMipmap(envUnfiltered->GetTextureID());
		}

		Ref<Shader> envFilteringShader = s_Data->ShaderLibrary->TryGet("EnvironmentMipFilter");
		Ref<CubeTexture> envFiltered = CubeTexture::Create(ImageFormat::RGBA32F, cubeMapSize, cubeMapSize, nullptr, cubeTextureProps);

		// Copy the Unfiltered cubemap image to the filtered cubemap image which will later be filtered and returned as the radiance map
		{
			glCopyImageSubData(
				// Src
				envUnfiltered->GetTextureID(), 
				GL_TEXTURE_CUBE_MAP, 
				0, 0, 0, 0, 
				// Dst
				envFiltered->GetTextureID(), 
				GL_TEXTURE_CUBE_MAP, 
				0, 0, 0, 0, 
				// Size
				envFiltered->GetWidth(), 
				envFiltered->GetHeight(), 
				6);
		}

		// Dispatch compute shader to filter the environment map and its mips based on a roughness value
		{
			AR_SCOPED_TIMER("Filtering CubeMap mips");
			AR_PROFILE_SCOPE("Filtering CubeMap mips");

			envFilteringShader->Bind();
			envUnfiltered->Bind(0); // Input texture

			const float mipAmount = (float)envFiltered->GetMipCount();
			const float deltaRoughness = 1.0f / glm::max(mipAmount - 1.0f, 1.0f);
			uint32_t size = cubeMapSize / 2;
			for (uint32_t level = 1; level < mipAmount; level++)
			{
				// Output texture
				glBindImageTexture(0, envFiltered->GetTextureID(), level, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA32F);

				envFilteringShader->SetUniform("u_Uniforms.Roughness", (float)level * deltaRoughness);

				const uint32_t numGroups = glm::max(1u, size / 32);
				envFilteringShader->Dispatch(numGroups, numGroups, 6);
				glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT | GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

				size /= 2;
			}
		}

		Ref<Shader> envIrradianceShader = s_Data->ShaderLibrary->Get("EnvironmentIrradiance");
		Ref<CubeTexture> irradianceMap = CubeTexture::Create(ImageFormat::RGBA32F, irradianceMapSize, irradianceMapSize, nullptr, cubeTextureProps);

		{
			AR_SCOPED_TIMER("Generating IrradianceMap");
			AR_PROFILE_SCOPE("Generating IrradianceMap");

			envIrradianceShader->Bind();
			envFiltered->Bind(0); // Input texture

			// Output texture
			glBindImageTexture(0, irradianceMap->GetTextureID(), 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA32F);

			envIrradianceShader->SetUniform("u_Uniforms.Samples", irradianceMapSamples);
			envIrradianceShader->Dispatch(irradianceMap->GetWidth() / 32, irradianceMap->GetHeight() / 32, 6);
			glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT | GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
			glGenerateTextureMipmap(irradianceMap->GetTextureID());
		}

		return { envFiltered, irradianceMap };
	}

	void Renderer::GenOrGetBRDFLutTexture()
	{
		constexpr const char* BRDFLutPath = "Resources/cache/renderer/BRDFLut.tga";
		if (std::filesystem::exists(BRDFLutPath))
		{
			AR_CORE_INFO_TAG("Renderer", "Found BRDF Loop up texture at: {0}", BRDFLutPath);

			TextureProperties BRDFProps = {};
			BRDFProps.SamplerWrap = TextureWrap::Clamp;
			BRDFProps.GenerateMips = true;
			s_Data->BRDFLutTexture = Texture2D::Create(BRDFLutPath, BRDFProps);
		}
		else
		{
			// Compute Cook-Torrance BRDF 2D LUT for split-sum approximation.
			AR_SCOPED_TIMER("Generating BRDFLut");
			AR_PROFILE_SCOPE("Generating BRDFLut");

			AR_CORE_INFO_TAG("Renderer", "Did not find BRDF Look up texture!");
			AR_CORE_INFO_TAG("Renderer", "Generating a BRDF Loop up texture...");

			Ref<Shader> BRDFShader = s_Data->ShaderLibrary->Get("BRDFLutGen");
			TextureProperties BRDFProps = {};
			BRDFProps.SamplerWrap = TextureWrap::Clamp;
			BRDFProps.GenerateMips = false; // Dont gen mips at first...
			Ref<Texture2D> BRDFTexture = Texture2D::Create(ImageFormat::RG16F, 256, 256, nullptr, BRDFProps);

			BRDFShader->Bind();
			glBindImageTexture(0, BRDFTexture->GetTextureID(), 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RG16F);
			BRDFShader->Dispatch(BRDFTexture->GetWidth() / 32, BRDFTexture->GetHeight() / 32, 1);
			glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT | GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
			glGenerateTextureMipmap(BRDFTexture->GetTextureID()); // Now gen mips
			
			// cache to disk...
			// TODO: Debug this shit...
			//void* data;
			//glGetTextureSubImage(BRDFTexture->GetTextureID(), 0, 0, 0, 0, 256, 256, 0, GL_RG, GL_FLOAT, Utils::GetImageMemorySize(ImageFormat::RG16F, 256, 256), data);
			// Utils::ImageLoader::WriteDataToTGAImage("Resources/cache/renderer/BRDFLut.tga", data, 256, 256, 3 /* 2?? */, false);
			//Utils::ImageLoader::WriteDataToPNGImage("Resources/cache/renderer/BRDFLut.tga", data, 256, 256, 3 /* 2?? */, false);

			s_Data->BRDFLutTexture = BRDFTexture;
		}
	}

	void Renderer::BeginScene(const Ref<EditorCamera>& camera)
	{
	}

	void Renderer::EndScene()
	{
	}

	void Renderer::OnWindowResize(uint32_t width, uint32_t height)
	{
		RenderCommand::SetViewport(0, 0, width, height);
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

	const Ref<Texture2D>& Renderer::GetBRDFLutTexture()
	{
		return s_Data->BRDFLutTexture;
	}

	const Ref<Texture2D>& Renderer::GetBlackTexture()
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