#include "Aurorapch.h"
#include "Renderer.h"

#include "Core/Application.h"
#include "Renderer2D.h"
#include "RenderCommand.h"
#include "Utils/ImageLoader.h"

#include <glad/glad.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/Importer.hpp>
#include "Utils/AssimpLogStream.h"
namespace Aurora {

	const unsigned int PBRImportFlags =
		aiProcess_CalcTangentSpace |
		aiProcess_Triangulate |
		aiProcess_SortByPType |
		aiProcess_PreTransformVertices |
		aiProcess_GenNormals |
		aiProcess_GenUVCoords |
		aiProcess_OptimizeMeshes |
		aiProcess_Debone |
		aiProcess_ValidateDataStructure;

	class PBRMesh
	{
	public:
		struct Vertex
		{
			glm::vec3 position;
			glm::vec3 normal;
			glm::vec3 tangent;
			glm::vec3 bitangent;
			glm::vec2 texcoord;
		};
		static_assert(sizeof(Vertex) == 14 * sizeof(float), "Wassup");
		static const int NumAttributes = 5;

		struct Face
		{
			uint32_t v1, v2, v3;
		};
		static_assert(sizeof(Face) == 3 * sizeof(uint32_t), "Wassup");

		static std::shared_ptr<PBRMesh> fromFile(const std::string& filename);
		static std::shared_ptr<PBRMesh> fromString(const std::string& data);

		const std::vector<Vertex>& vertices() const { return m_vertices; }
		const std::vector<Face>& faces() const { return m_faces; }

	private:
		PBRMesh(const struct aiMesh* mesh);

		std::vector<Vertex> m_vertices;
		std::vector<Face> m_faces;
	};

	PBRMesh::PBRMesh(const aiMesh* mesh)
	{
		assert(mesh->HasPositions());
		assert(mesh->HasNormals());

		m_vertices.reserve(mesh->mNumVertices);
		for (size_t i = 0; i < m_vertices.capacity(); ++i) {
			Vertex vertex;
			vertex.position = { mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z };
			vertex.normal = { mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z };
			if (mesh->HasTangentsAndBitangents()) {
				vertex.tangent = { mesh->mTangents[i].x, mesh->mTangents[i].y, mesh->mTangents[i].z };
				vertex.bitangent = { mesh->mBitangents[i].x, mesh->mBitangents[i].y, mesh->mBitangents[i].z };
			}
			if (mesh->HasTextureCoords(0)) {
				vertex.texcoord = { mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y };
			}
			m_vertices.push_back(vertex);
		}

		m_faces.reserve(mesh->mNumFaces);
		for (size_t i = 0; i < m_faces.capacity(); ++i) {
			assert(mesh->mFaces[i].mNumIndices == 3);
			m_faces.push_back({ mesh->mFaces[i].mIndices[0], mesh->mFaces[i].mIndices[1], mesh->mFaces[i].mIndices[2] });
		}
	}

	std::shared_ptr<PBRMesh> PBRMesh::fromFile(const std::string& filename)
	{
		AssimpLogStream::Init();

		std::printf("Loading mesh: %s\n", filename.c_str());

		std::shared_ptr<PBRMesh> mesh;
		Assimp::Importer importer;

		const aiScene* scene = importer.ReadFile(filename, PBRImportFlags);
		if (scene && scene->HasMeshes()) {
			mesh = std::shared_ptr<PBRMesh>(new PBRMesh{ scene->mMeshes[0] });
		}
		else {
			throw std::runtime_error("Failed to load mesh file: " + filename);
		}
		return mesh;
	}

	struct MeshBuffer
	{
		MeshBuffer() : vbo(0), ibo(0), vao(0), numElements(0) {}
		GLuint vbo, ibo, vao;
		GLuint numElements;
	};

	static MeshBuffer createMeshBuffer(const std::shared_ptr<class PBRMesh>& mesh)
	{
		MeshBuffer buffer;
		buffer.numElements = static_cast<GLuint>(mesh->faces().size()) * 3; // Since faces are triangles with 3 indices each

		const size_t vertexDataSize = mesh->vertices().size() * sizeof(PBRMesh::Vertex);
		const size_t indexDataSize = mesh->faces().size() * sizeof(PBRMesh::Face);

		glCreateBuffers(1, &buffer.vbo);
		glNamedBufferStorage(buffer.vbo, vertexDataSize, reinterpret_cast<const void*>(&mesh->vertices()[0]), 0);
		glCreateBuffers(1, &buffer.ibo);
		glNamedBufferStorage(buffer.ibo, indexDataSize, reinterpret_cast<const void*>(&mesh->faces()[0]), 0);

		glCreateVertexArrays(1, &buffer.vao);
		glVertexArrayElementBuffer(buffer.vao, buffer.ibo);
		for (int i = 0; i < PBRMesh::NumAttributes; ++i) {
			glVertexArrayVertexBuffer(buffer.vao, i, buffer.vbo, i * sizeof(glm::vec3), sizeof(PBRMesh::Vertex));
			glEnableVertexArrayAttrib(buffer.vao, i);
			glVertexArrayAttribFormat(buffer.vao, i, i == (PBRMesh::NumAttributes - 1) ? 2 : 3, GL_FLOAT, GL_FALSE, 0);
			glVertexArrayAttribBinding(buffer.vao, i, i);
		}
		return buffer;
	}

	struct RendererData
	{
		Scope<ShaderLibrary> ShaderLibrary;
		Ref<Texture2D> WhiteTexture;
		Ref<Texture2D> BlackTexture;
		Ref<Texture2D> BRDFLutTexture;
		Ref<CubeTexture> BlackCubeTexture;
		Ref<Environment> NullEnvironment;

		MeshBuffer m_Skybox;

		Ref<VertexArray> FullScreenQuadVertexArray;
		Ref<VertexBuffer> FullScreenQuadVertexBuffer;
		Ref<IndexBuffer> FullScreenQuadIndexBuffer;

		RendererConfig Settings;
	};

	static RendererData* s_Data = nullptr;

	void Renderer::Init(const RendererConfig& configuration)
	{
		RendererCapabilities::Init();

		// Init OpenGL Global State variables
		{
			glEnable(GL_DEPTH_TEST);
			glDepthFunc(GL_LESS);

			glEnable(GL_MULTISAMPLE);
			glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
			// glEnable(GL_FRAMEBUFFER_SRGB);
			// Different color space

			glEnable(GL_CULL_FACE);
			glCullFace(GL_BACK);
			glFrontFace(GL_CCW);

			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		}

		s_Data = new RendererData();

		s_Data->ShaderLibrary = ShaderLibrary::Create();
		s_Data->Settings.EnvironmentMapResolution = configuration.EnvironmentMapResolution;
		s_Data->Settings.IrradianceMapComputeSamples = configuration.IrradianceMapComputeSamples;

		ShaderProperties auroraPBRStaticProps = {};
		auroraPBRStaticProps.Name = "AuroraPBRStatic";
		auroraPBRStaticProps.AssetPath = "Resources/shaders/AuroraPBRStatic.glsl";
		s_Data->ShaderLibrary->Load(auroraPBRStaticProps);

		ShaderProperties BRDFLutShaderProps = {};
		BRDFLutShaderProps.Name = "BRDFLutGen";
		BRDFLutShaderProps.AssetPath = "Resources/shaders/BRDFLutGen.glsl";
		BRDFLutShaderProps.Type = ShaderType::Compute;
		s_Data->ShaderLibrary->Load(BRDFLutShaderProps);

		ShaderProperties texturePassShaderProps = {};
		texturePassShaderProps.Name = "TexturePass";
		texturePassShaderProps.AssetPath = "Resources/shaders/DefaultTextureShader.glsl";
		s_Data->ShaderLibrary->Load(texturePassShaderProps);

		ShaderProperties environmentIrradiance = {};
		environmentIrradiance.Name = "EnvironmentIrradiance";
		environmentIrradiance.AssetPath = "Resources/shaders/EnvironmentIrradiance.glsl";
		environmentIrradiance.Type = ShaderType::Compute;
		s_Data->ShaderLibrary->Load(environmentIrradiance);

		ShaderProperties environmentLevelFilter = {};
		environmentLevelFilter.Name = "EnvironmentMipFilter";
		environmentLevelFilter.AssetPath = "Resources/shaders/EnvironmentLevelFilter.glsl";
		environmentLevelFilter.Type = ShaderType::Compute;
		s_Data->ShaderLibrary->Load(environmentLevelFilter);

		ShaderProperties equirectangularToCubeMapProps = {};
		equirectangularToCubeMapProps.Name = "EquirectToCubeMap";
		equirectangularToCubeMapProps.AssetPath = "Resources/shaders/EquiRectangularToCubeMap.glsl";
		equirectangularToCubeMapProps.Type = ShaderType::Compute;
		s_Data->ShaderLibrary->Load(equirectangularToCubeMapProps);

		ShaderProperties gridProps = {};
		gridProps.Name = "Grid";
		gridProps.AssetPath = "Resources/shaders/Grid.glsl";
		gridProps.Type = ShaderType::TwoStageVertFrag;
		s_Data->ShaderLibrary->Load(gridProps);

		ShaderProperties preethamSkyProps = {};
		preethamSkyProps.Name = "PreethamSky";
		preethamSkyProps.AssetPath = "Resources/shaders/PreethamSky.glsl";
		preethamSkyProps.Type = ShaderType::Compute;
		s_Data->ShaderLibrary->Load(preethamSkyProps);

		ShaderProperties renderer2DCircleProps = {};
		renderer2DCircleProps.Name = "Renderer2DCircle";
		renderer2DCircleProps.AssetPath = "Resources/shaders/Renderer2D_Circle.glsl";
		s_Data->ShaderLibrary->Load(renderer2DCircleProps);

		ShaderProperties renderer2DLineShaderProps = {};
		renderer2DLineShaderProps.Name = "Renderer2DLine";
		renderer2DLineShaderProps.AssetPath = "Resources/shaders/Renderer2D_Line.glsl";
		s_Data->ShaderLibrary->Load(renderer2DLineShaderProps);

		ShaderProperties renderer2DQuadShaderProps = {};
		renderer2DQuadShaderProps.Name = "Renderer2DQuad";
		renderer2DQuadShaderProps.AssetPath = "Resources/shaders/Renderer2D_Quad.glsl";
		s_Data->ShaderLibrary->Load(renderer2DQuadShaderProps);

		ShaderProperties skyBoxProps = {};
		skyBoxProps.Name = "Skybox";
		skyBoxProps.AssetPath = "Resources/shaders/Skybox.glsl";
		s_Data->ShaderLibrary->Load(skyBoxProps);

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

		// Create FullScreen Quad
		struct QuadVertex
		{
			glm::vec3 Position;
			glm::vec2 TexCoord;
		};

		QuadVertex data[4];

		data[0].Position = glm::vec3(-1.0f, -1.0f, 0.1f);
		data[0].TexCoord = glm::vec2(0, 0);

		data[1].Position = glm::vec3(1.0f, -1.0f, 0.1f);
		data[1].TexCoord = glm::vec2(1, 0);

		data[2].Position = glm::vec3(1.0f, 1.0f, 0.1f);
		data[2].TexCoord = glm::vec2(1, 1);

		data[3].Position = glm::vec3(-1.0f, 1.0f, 0.1f);
		data[3].TexCoord = glm::vec2(0, 1);

		s_Data->FullScreenQuadVertexArray = VertexArray::Create();
		s_Data->FullScreenQuadVertexBuffer = VertexBuffer::Create((void*)data, 4 * sizeof(QuadVertex));
		s_Data->FullScreenQuadVertexBuffer->SetLayout({
			{ ShaderDataType::Float3, "a_Position" },
			{ ShaderDataType::Float2, "a_TexCoord" }
		});
		uint32_t indices[6] = { 0, 1, 2, 2, 3, 0 };
		//uint32_t indices[6] = { 0, 2, 1, 2, 0, 3 };
		s_Data->FullScreenQuadIndexBuffer = IndexBuffer::Create(indices, 6 * sizeof(uint32_t));
		s_Data->FullScreenQuadVertexArray->AddVertexBuffer(s_Data->FullScreenQuadVertexBuffer);
		s_Data->FullScreenQuadVertexArray->SetIndexBuffer(s_Data->FullScreenQuadIndexBuffer);

		s_Data->m_Skybox = createMeshBuffer(PBRMesh::fromFile("Resources/meshes/Default/skybox.obj"));
	}

	void Renderer::ShutDown()
	{
		 RendererCapabilities::ShutDown();

		 delete s_Data;
	}

	void Renderer::SetSceneEnvironment(Ref<SceneRenderer> sceneRenderer, Ref<Environment> environment)
	{
		AR_PROFILE_FUNCTION();

		if (!environment)
			environment = s_Data->NullEnvironment;

		// TODO: Does this need to bing the shader?

		const Ref<Shader>& shader = s_Data->ShaderLibrary->Get("AuroraPBRStatic");

		const ShaderResourceDeclaration* radianceResource = shader->GetShaderResource("u_EnvRadianceTexture");
		if (radianceResource)
		{
			Ref<CubeTexture> radianceMap = environment->RadianceMap;
			radianceMap->Bind(radianceResource->GetRegister());
		}

		const ShaderResourceDeclaration* irradianceResource = shader->GetShaderResource("u_EnvIrradianceTexture");
		if (irradianceResource)
		{
			Ref<CubeTexture> irradianceMap = environment->IrradianceMap;
			irradianceMap->Bind(irradianceResource->GetRegister());
		}

		const ShaderResourceDeclaration* brdfResource = shader->GetShaderResource("u_BRDFLutTexture");
		if (brdfResource)
		{
			Ref<Texture2D> BRDFTexture = s_Data->BRDFLutTexture;
			BRDFTexture->Bind(brdfResource->GetRegister());
		}
	}

	std::pair<Ref<CubeTexture>, Ref<CubeTexture>> Renderer::CreateEnvironmentMap(const std::string& filePath)
	{
		AR_PROFILE_FUNCTION();

		const uint32_t cubeMapSize = s_Data->Settings.EnvironmentMapResolution;
		constexpr uint32_t irradianceMapSize = 32;
		const uint32_t irradianceMapSamples = s_Data->Settings.IrradianceMapComputeSamples;

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
		Ref<CubeTexture> envFiltered = CubeTexture::Create(ImageFormat::RGBA32F, cubeMapSize, cubeMapSize, nullptr, cubeTextureProps, filePath);

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
		Ref<CubeTexture> irradianceMap = CubeTexture::Create(ImageFormat::RGBA32F, irradianceMapSize, irradianceMapSize, nullptr, cubeTextureProps, filePath);

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

	Ref<CubeTexture> Renderer::CreatePreethamSky(float turbidity, float azimuth, float inclination)
	{
		AR_PROFILE_FUNCTION();

		const uint32_t cubeMapSize = 1024; // TODO: This should be a rendererConfig global setting and could be set from editor
		
		TextureProperties cubeMapProps = {};
		cubeMapProps.GenerateMips = true;

		Ref<Shader> preethamSkySader = s_Data->ShaderLibrary->Get("PreethamSky");
		Ref<CubeTexture> environmentMap = CubeTexture::Create(ImageFormat::RGBA32F, cubeMapSize, cubeMapSize, nullptr, cubeMapProps);

		{
			AR_SCOPED_TIMER("Generating PreethamSky");
			AR_PROFILE_SCOPE("Generating PreethamSky");

			preethamSkySader->Bind();
			preethamSkySader->SetUniform("u_Uniforms.TurbidityAzimuthInclination", glm::vec3{ turbidity, azimuth, inclination });
			glBindImageTexture(0, environmentMap->GetTextureID(), 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA32F);
			preethamSkySader->Dispatch(cubeMapSize / 32, cubeMapSize / 32, 6);
			glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT | GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
			glGenerateTextureMipmap(environmentMap->GetTextureID());
		}

		return environmentMap;
	}

	void Renderer::GenOrGetBRDFLutTexture()
	{
		constexpr const char* BRDFLutPath = "Resources/cache/renderer/BRDFLut.tga";
		if (std::filesystem::exists(BRDFLutPath))
		{
			AR_CORE_INFO_TAG("Renderer", "Found BRDF Look up texture at: {0}", BRDFLutPath);

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

			AR_CORE_WARN_TAG("Renderer", "Did not find BRDF Look up texture!");
			AR_CORE_INFO_TAG("Renderer", "Generating BRDF Look up texture...");

			Ref<Shader> BRDFShader = s_Data->ShaderLibrary->Get("BRDFLutGen");
			TextureProperties BRDFProps = {};
			BRDFProps.SamplerWrap = TextureWrap::Clamp;
			BRDFProps.GenerateMips = false; // Dont gen mips at first...
			Ref<Texture2D> BRDFTexture = Texture2D::Create(ImageFormat::RG16F, 256, 256, nullptr, BRDFProps);

			BRDFShader->Bind();
			glBindImageTexture(0, BRDFTexture->GetTextureID(), 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RG16F);
			BRDFShader->Dispatch(BRDFTexture->GetWidth() / 32, BRDFTexture->GetHeight() / 32, 1);
			glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT | GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
			// glGenerateTextureMipmap(BRDFTexture->GetTextureID()); // Now gen mips
			
			// Cache image to disk
			{
				Buffer buff(Utils::GetImageMemorySize(ImageFormat::RG16F, 256, 256));
				glGetTextureImage(BRDFTexture->GetTextureID(), 0, GL_RGB, GL_UNSIGNED_BYTE, (GLsizei)buff.Size, buff.Data);

				if(!std::filesystem::exists("Resources/cache/renderer"))
					std::filesystem::create_directories("Resources/cache/renderer");
				Utils::ImageLoader::WriteDataToTGAImage(BRDFLutPath, buff.Data, 256, 256, 3);
			}
			
			s_Data->BRDFLutTexture = BRDFTexture;
		}
	}

	void Renderer::BeginScene(const Ref<EditorCamera>& camera)
	{
	}

	void Renderer::EndScene()
	{
	}

	void Renderer::BeginRenderPass(const Ref<RenderPass>& renderPass)
	{
		renderPass->GetSpecification().TargetFramebuffer->Bind();
	}

	void Renderer::EndRenderPass(const Ref<RenderPass>& renderPass)
	{
		renderPass->GetSpecification().TargetFramebuffer->UnBind();
	}

	void Renderer::SubmitFullScreenQuad(const Ref<Material>& material)
	{
		if (!material->HasFlag(MaterialFlag::DepthTest))
			glDisable(GL_DEPTH_TEST);

		if (material->HasFlag(MaterialFlag::TwoSided))
			glDisable(GL_CULL_FACE);

		material->SetUpForRendering();

		s_Data->FullScreenQuadVertexArray->Bind();
		uint32_t indexCount = s_Data->FullScreenQuadIndexBuffer->GetCount();
		glDrawElementsBaseVertex(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, nullptr, 0);

		if (material->HasFlag(MaterialFlag::TwoSided))
			glEnable(GL_CULL_FACE);

		if (!material->HasFlag(MaterialFlag::DepthTest))
			glEnable(GL_DEPTH_TEST);
	}

	// This is mainly used for the grid shader, Works like SubmitFullScreenQuad however takes in a transform
	void Renderer::RenderQuad(const Ref<Material>& material, const glm::mat4& transform)
	{
		if (!material->HasFlag(MaterialFlag::DepthTest))
			glDisable(GL_DEPTH_TEST);

		if (material->HasFlag(MaterialFlag::TwoSided))
			glDisable(GL_CULL_FACE);

		material->Set("u_Renderer.Transform", transform);
		material->SetUpForRendering();

		s_Data->FullScreenQuadVertexArray->Bind();
		uint32_t indexCount = s_Data->FullScreenQuadIndexBuffer->GetCount();
		glDrawElementsBaseVertex(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, nullptr, 0);

		if (material->HasFlag(MaterialFlag::TwoSided))
			glEnable(GL_CULL_FACE);

		if (!material->HasFlag(MaterialFlag::DepthTest))
			glEnable(GL_DEPTH_TEST);
	}

	// This is for rendering whatever the batch renderer wants
	void Renderer::RenderGeometry(const Ref<UniformBuffer>& ubo, const Ref<StorageBuffer>& ssbo, const Ref<Material>& mat, const Ref<VertexArray>& vao, uint32_t indexCount)
	{
		if (indexCount == 0)
			indexCount = vao->GetIndexBuffer()->GetCount();

		mat->SetUpForRendering();
		vao->Bind();
		glDrawElementsBaseVertex(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, nullptr, 0);
	}

	// TODO: Work on rendering static meshes
	void Renderer::RenderStaticMesh(const Ref<StaticMesh>& mesh, const Ref<MaterialTable>& material, const glm::mat4& tranform)
	{
		AR_CORE_CHECK(mesh);
		AR_CORE_CHECK(material);

		Ref<MeshSource> meshSource = mesh->GetMeshSource();

		//Ref<VertexBuffer> vertexBuffer = meshSource->GetVertexBuffer();
		//vertexBuffer->Bind();

		//Ref<IndexBuffer> indexBuffer = meshSource->GetIndexBuffer();
		//indexBuffer->Bind();

		Ref<VertexArray> vertexArray = meshSource->GetVertexArray();
		vertexArray->Bind();

		auto& meshMatTable = mesh->GetMaterials();
		uint32_t materialCount = meshMatTable->GetMaterialCount();
		std::vector<Ref<MaterialAsset>> renderMaterials(materialCount);
		for (uint32_t i = 0; i < materialCount; i++)
		{
			if (material->HasMaterial(i))
				renderMaterials[i] = material->GetMaterial(i);
			else
				renderMaterials[i] = meshMatTable->GetMaterial(i);

			//Ref<Material> materialToRender = renderMaterials[i]->GetMaterial();
			//// TODO: ??
			//materialToRender->SetUpForRendering();
		}

		glDisable(GL_CULL_FACE);
		const auto& meshSourceSubmeshes = meshSource->GetSubMeshes();
		const auto& subMeshes = mesh->GetSubMeshes();
		for (uint32_t subMeshIndex : subMeshes)
		{
			const SubMesh& subMesh = meshSourceSubmeshes[subMeshIndex];
			auto& finalmaterial = renderMaterials[subMesh.MaterialIndex]->GetMaterial();
			
			//meshSource->GetVertexArray()->Bind();
			//meshSource->GetIndexBuffer()->Bind();
			finalmaterial->SetUpForRendering();
			glDrawElementsBaseVertex(GL_TRIANGLES, subMesh.IndexCount, GL_UNSIGNED_INT, (void*)(uint64_t)subMesh.BaseIndex, subMesh.BaseVertex);
		}
		glEnable(GL_CULL_FACE);
	}

	void Renderer::OnWindowResize(uint32_t width, uint32_t height)
	{
		glViewport(0, 0, width, height);
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

	RendererConfig& Renderer::GetConfig()
	{
		return s_Data->Settings;
	}

}