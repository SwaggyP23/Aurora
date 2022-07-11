#include "Aurorapch.h"
#include "Renderer3D.h"

namespace Aurora {

	Ref<Texture> Renderer3D::m_ContainerTexture;

	struct QuadVertex
	{
		glm::vec3 Position;
		glm::vec4 Color;
		glm::vec3 Normals;
		glm::vec2 TexCoords;
		float TextureIndex;
		float TilingFactor;
		int light;
	};

	// So for my laptop, it can not hit 60 fps is the MaxQuads is more than 1.5k since that is alot of memory to be transfered in one go
	// from the CPU to the GPU, even if you are only rendering like 15 quads it will not peak in fps since, again, the memory is to big!
	// Therefore for lowerend laptops, it is better to keep the MaxQuads under the 1.5k mark.

	struct RendererData
	{
		static const size_t MaxQuads = 1000;
		static const size_t MaxVertices = MaxQuads * 24;
		static const size_t MaxIndices = MaxQuads * 36;
		static const size_t MaxTextureSlots = 16; // 16 for Luna and 32 for SandBox which is stupid
		//const size_t MaxTextureSlots   = RendererProperties::GetRendererProperties()->TextureSlots;

		Ref<VertexArray> QuadVertexArray;
		Ref<VertexBuffer> QuadVertexBuffer;
		Ref<Shader> QuadShader;
		Ref<Texture> WhiteTex;

		uint32_t QuadIndexCount = 0;
		QuadVertex* QuadVertexBufferBase = nullptr; // This is to keep track of the base of memory allocations
		QuadVertex* QuadVertexBufferPtr = nullptr;

		// Here the identifier will become an asset handle if i ever implement it
		std::array<Ref<Texture>, MaxTextureSlots> TextureSlots;
		uint32_t TextureSlotIndex = 1; // 0 is the white texture

		glm::vec4 QuadVertexPositions[24];
		glm::vec3 QuadNormalPositions[24];

		uint32_t quadVertexCount = 24;

		glm::vec2 textureCoords[24];

		Renderer3D::Statistics Stats;
	};

	static RendererData s_Data;

	void Renderer3D::Init()
	{
		PROFILE_FUNCTION();

		s_Data.QuadVertexArray = VertexArray::Create();

		s_Data.QuadVertexBuffer = VertexBuffer::Create((uint32_t)s_Data.MaxVertices * sizeof(QuadVertex));
		s_Data.QuadVertexBuffer->setLayout({
			{ ShaderDataType::Float3, "a_Position"     },
			{ ShaderDataType::Float4, "a_Color"        },
			{ ShaderDataType::Float3, "a_Normals"      },
			{ ShaderDataType::Float2, "a_TexCoord"     },
			{ ShaderDataType::Float,  "a_TexIndex"     },
			{ ShaderDataType::Float,  "a_TilingFactor" },
			{ ShaderDataType::Int,    "a_Light"        }
		});
		s_Data.QuadVertexArray->addVertexBuffer(s_Data.QuadVertexBuffer);

		s_Data.QuadVertexBufferBase = new QuadVertex[s_Data.MaxVertices];

		uint32_t* quadIndices = new uint32_t[s_Data.MaxIndices];

		uint32_t offset = 0;
		for (uint32_t i = 0; i < s_Data.MaxIndices; i += 36)
		{
			quadIndices[i + 0] = offset + 0;
			quadIndices[i + 1] = offset + 1;
			quadIndices[i + 2] = offset + 2;
			quadIndices[i + 3] = offset + 2;
			quadIndices[i + 4] = offset + 3;
			quadIndices[i + 5] = offset + 0;

			quadIndices[i + 6] = offset + 4;
			quadIndices[i + 7] = offset + 5;
			quadIndices[i + 8] = offset + 6;
			quadIndices[i + 9] = offset + 6;
			quadIndices[i + 10] = offset + 7;
			quadIndices[i + 11] = offset + 4;

			quadIndices[i + 12] = offset + 8;
			quadIndices[i + 13] = offset + 9;
			quadIndices[i + 14] = offset + 10;
			quadIndices[i + 15] = offset + 10;
			quadIndices[i + 16] = offset + 11;
			quadIndices[i + 17] = offset + 8;

			quadIndices[i + 18] = offset + 12;
			quadIndices[i + 19] = offset + 13;
			quadIndices[i + 20] = offset + 14;
			quadIndices[i + 21] = offset + 14;
			quadIndices[i + 22] = offset + 15;
			quadIndices[i + 23] = offset + 12;

			quadIndices[i + 24] = offset + 16;
			quadIndices[i + 25] = offset + 17;
			quadIndices[i + 26] = offset + 18;
			quadIndices[i + 27] = offset + 18;
			quadIndices[i + 28] = offset + 19;
			quadIndices[i + 29] = offset + 16;

			quadIndices[i + 30] = offset + 20;
			quadIndices[i + 31] = offset + 21;
			quadIndices[i + 32] = offset + 22;
			quadIndices[i + 33] = offset + 22;
			quadIndices[i + 34] = offset + 23;
			quadIndices[i + 35] = offset + 20;

			offset += 24;
		}

		Ref<IndexBuffer> quadIB = IndexBuffer::Create(quadIndices, (uint32_t)s_Data.MaxIndices);
		s_Data.QuadVertexArray->setIndexBuffer(quadIB);
		delete[] quadIndices;

		s_Data.WhiteTex = Texture::Create(1, 1);
		uint32_t whiteTextureData = 0xffffffff;
		s_Data.WhiteTex->setData(&whiteTextureData, sizeof(uint32_t));

		int samplers[s_Data.MaxTextureSlots];
		for (int i = 0; i < s_Data.MaxTextureSlots; i++)
			samplers[i] = i;
		// This is the sampler that will be submitted to OpenGL and in which OpenGL will be sampling the textures from according to the passed index

		s_Data.QuadShader = Shader::Create("resources/shaders/MainShader.glsl");
		s_Data.QuadShader->bind();
		s_Data.QuadShader->setUniformArrayi("u_Textures", samplers, s_Data.MaxTextureSlots);

		s_Data.TextureSlots[0] = s_Data.WhiteTex; // index 0 is for the white texture.

		s_Data.QuadVertexPositions[0] = { -0.5f, -0.5f, -0.5f, 1.0f };
		s_Data.QuadVertexPositions[1] = { 0.5f, -0.5f, -0.5f, 1.0f };
		s_Data.QuadVertexPositions[2] = { 0.5f,  0.5f, -0.5f, 1.0f };
		s_Data.QuadVertexPositions[3] = { -0.5f,  0.5f, -0.5f, 1.0f };

		s_Data.QuadVertexPositions[4] = { -0.5f, -0.5f,  0.5f, 1.0f };
		s_Data.QuadVertexPositions[5] = { 0.5f, -0.5f,  0.5f, 1.0f };
		s_Data.QuadVertexPositions[6] = { 0.5f,  0.5f,  0.5f, 1.0f };
		s_Data.QuadVertexPositions[7] = { -0.5f,  0.5f,  0.5f, 1.0f };

		s_Data.QuadVertexPositions[8] = { -0.5f, -0.5f, -0.5f, 1.0f };
		s_Data.QuadVertexPositions[9] = { -0.5f, -0.5f,  0.5f, 1.0f };
		s_Data.QuadVertexPositions[10] = { -0.5f,  0.5f,  0.5f, 1.0f };
		s_Data.QuadVertexPositions[11] = { -0.5f,  0.5f, -0.5f, 1.0f };

		s_Data.QuadVertexPositions[12] = { 0.5f, -0.5f, -0.5f, 1.0f };
		s_Data.QuadVertexPositions[13] = { 0.5f, -0.5f,  0.5f, 1.0f };
		s_Data.QuadVertexPositions[14] = { 0.5f,  0.5f,  0.5f, 1.0f };
		s_Data.QuadVertexPositions[15] = { 0.5f,  0.5f, -0.5f, 1.0f };

		s_Data.QuadVertexPositions[16] = { -0.5f, -0.5f, -0.5f, 1.0f };
		s_Data.QuadVertexPositions[17] = { 0.5f, -0.5f, -0.5f, 1.0f };
		s_Data.QuadVertexPositions[18] = { 0.5f, -0.5f,  0.5f, 1.0f };
		s_Data.QuadVertexPositions[19] = { -0.5f, -0.5f,  0.5f, 1.0f };

		s_Data.QuadVertexPositions[20] = { -0.5f,  0.5f, -0.5f, 1.0f };
		s_Data.QuadVertexPositions[21] = { 0.5f,  0.5f, -0.5f, 1.0f };
		s_Data.QuadVertexPositions[22] = { 0.5f,  0.5f,  0.5f, 1.0f };
		s_Data.QuadVertexPositions[23] = { -0.5f,  0.5f,  0.5f, 1.0f };

		s_Data.textureCoords[0] = { 1.0f, 0.0f };
		s_Data.textureCoords[1] = { 0.0f, 0.0f };
		s_Data.textureCoords[2] = { 0.0f, 1.0f };
		s_Data.textureCoords[3] = { 1.0f, 1.0f };

		s_Data.textureCoords[4] = { 0.0f, 0.0f };
		s_Data.textureCoords[5] = { 1.0f, 0.0f };
		s_Data.textureCoords[6] = { 1.0f, 1.0f };
		s_Data.textureCoords[7] = { 0.0f, 1.0f };

		s_Data.textureCoords[8] = { 0.0f, 0.0f };
		s_Data.textureCoords[9] = { 1.0f, 0.0f };
		s_Data.textureCoords[10] = { 1.0f, 1.0f };
		s_Data.textureCoords[11] = { 0.0f, 1.0f };

		s_Data.textureCoords[12] = { 1.0f, 0.0f };
		s_Data.textureCoords[13] = { 0.0f, 0.0f };
		s_Data.textureCoords[14] = { 0.0f, 1.0f };
		s_Data.textureCoords[15] = { 1.0f, 1.0f };

		s_Data.textureCoords[16] = { 0.0f, 1.0f };
		s_Data.textureCoords[17] = { 1.0f, 1.0f };
		s_Data.textureCoords[18] = { 1.0f, 0.0f };
		s_Data.textureCoords[19] = { 0.0f, 0.0f };

		s_Data.textureCoords[20] = { 0.0f, 0.0f };
		s_Data.textureCoords[21] = { 1.0f, 0.0f };
		s_Data.textureCoords[22] = { 1.0f, 1.0f };
		s_Data.textureCoords[23] = { 0.0f, 1.0f };

		s_Data.QuadNormalPositions[0] = { 0.0f,  0.0f, -1.0f };
		s_Data.QuadNormalPositions[1] = { 0.0f,  0.0f, -1.0f };
		s_Data.QuadNormalPositions[2] = { 0.0f,  0.0f, -1.0f };
		s_Data.QuadNormalPositions[3] = { 0.0f,  0.0f, -1.0f };

		s_Data.QuadNormalPositions[4] = { 0.0f,  0.0f,  1.0f };
		s_Data.QuadNormalPositions[5] = { 0.0f,  0.0f,  1.0f };
		s_Data.QuadNormalPositions[6] = { 0.0f,  0.0f,  1.0f };
		s_Data.QuadNormalPositions[7] = { 0.0f,  0.0f,  1.0f };

		s_Data.QuadNormalPositions[8] = { -1.0f,  0.0f,  0.0f };
		s_Data.QuadNormalPositions[9] = { -1.0f,  0.0f,  0.0f };
		s_Data.QuadNormalPositions[10] = { -1.0f,  0.0f,  0.0f };
		s_Data.QuadNormalPositions[11] = { -1.0f,  0.0f,  0.0f };

		s_Data.QuadNormalPositions[12] = { 1.0f,  0.0f,  0.0f };
		s_Data.QuadNormalPositions[13] = { 1.0f,  0.0f,  0.0f };
		s_Data.QuadNormalPositions[14] = { 1.0f,  0.0f,  0.0f };
		s_Data.QuadNormalPositions[15] = { 1.0f,  0.0f,  0.0f };

		s_Data.QuadNormalPositions[16] = { 0.0f, -1.0f,  0.0f };
		s_Data.QuadNormalPositions[17] = { 0.0f, -1.0f,  0.0f };
		s_Data.QuadNormalPositions[18] = { 0.0f, -1.0f,  0.0f };
		s_Data.QuadNormalPositions[19] = { 0.0f, -1.0f,  0.0f };

		s_Data.QuadNormalPositions[20] = { 0.0f,  1.0f,  0.0f };
		s_Data.QuadNormalPositions[21] = { 0.0f,  1.0f,  0.0f };
		s_Data.QuadNormalPositions[22] = { 0.0f,  1.0f,  0.0f };
		s_Data.QuadNormalPositions[23] = { 0.0f,  1.0f,  0.0f };
	}

	void Renderer3D::ShutDown()
	{
		delete[] s_Data.QuadVertexBufferBase;
	}

	void Renderer3D::BeginScene(const Ref<EditorCamera>& camera)
	{
		PROFILE_FUNCTION();

		s_Data.QuadShader->bind();
		//s_Data.QuadShader->setUniform3f("material.diffuse", color * glm::vec3(0.2f));
		s_Data.QuadShader->setUniform1i("material.specular", 2);
		s_Data.QuadShader->setUniform1f("material.shininess", 50.0f);
		s_Data.QuadShader->setUniform3f("light.Position", { 1.2f, 3.0f, 2.0f });
		s_Data.QuadShader->setUniform3f("light.Direction", { -0.2f, -1.0f, -0.3f });
		s_Data.QuadShader->setUniform3f("light.Ambient", { 0.2f, 0.2f, 0.2f });
		s_Data.QuadShader->setUniform3f("light.Diffuse", { 0.5f, 0.5f, 0.5f });
		s_Data.QuadShader->setUniform3f("light.Specular", glm::vec3(1.0f));
		s_Data.QuadShader->setUniform1f("light.Constant", 1.0f);
		s_Data.QuadShader->setUniform1f("light.Linear", 0.09f);
		s_Data.QuadShader->setUniform1f("light.Quadratic", 0.032f);
		s_Data.QuadShader->setUniform3f("u_ViewPosition", camera->GetPosition());
		s_Data.QuadShader->setUniformMat4("u_ViewProjmatrix", camera->GetViewProjection());
		
		StartBatch();
	}

	void Renderer3D::BeginScene(const Ref<OrthoGraphicCamera>& camera)
	{
		PROFILE_FUNCTION();

		s_Data.QuadShader->bind();
		s_Data.QuadShader->setUniform3f("u_ViewPosition", camera->GetPosition());
		s_Data.QuadShader->setUniformMat4("u_ViewProjmatrix", camera->GetViewProjection());

		StartBatch();
	}

	void Renderer3D::EndScene()
	{
		PROFILE_FUNCTION();

		Flush();
	}

	void Renderer3D::StartBatch()
	{
		s_Data.QuadIndexCount = 0;
		s_Data.QuadVertexBufferPtr = s_Data.QuadVertexBufferBase;

		s_Data.TextureSlotIndex = 1;
	}

	void Renderer3D::Flush()
	{
		if (s_Data.QuadIndexCount)
		{
			// Casting to one byte to actually do a correct calculation, otherwise it tells you the number of elements only since these are QuadVertex.
			uint32_t dataSize = (uint32_t)((uint8_t*)s_Data.QuadVertexBufferPtr - (uint8_t*)s_Data.QuadVertexBufferBase);
			s_Data.QuadVertexBuffer->SetData(s_Data.QuadVertexBufferBase, dataSize);
			// Bind Textures
			for (uint32_t i = 0; i < s_Data.TextureSlotIndex; i++)
				s_Data.TextureSlots[i]->bind(i);

			s_Data.QuadShader->bind();
			s_Data.QuadShader->setUniform3f("u_SourcePos", { 1.0, 12.0f, 17.0f });
			RenderCommand::DrawIndexed(s_Data.QuadVertexArray, s_Data.QuadIndexCount);

			s_Data.Stats.DrawCalls++;
		}
	}

	void Renderer3D::NextBatch()
	{
		Flush();
		StartBatch();
	}

	void Renderer3D::DrawQuad(const glm::vec3& position, const glm::vec3& scale, const glm::vec4& color, int light)
	{
		PROFILE_FUNCTION();

		if (s_Data.QuadIndexCount >= RendererData::MaxIndices)
			NextBatch();

		const float whiteTexIndex = 0.0f; // White texture.
		const float TilingFactor = 1.0f; // TilingFactor.

		glm::mat4 transform = glm::translate(glm::mat4(1.0f), position)
			* glm::scale(glm::mat4(1.0f), scale);

		glm::mat3 normalMat = glm::mat3(glm::transpose(glm::inverse(transform)));

		for (uint32_t i = 0; i < s_Data.quadVertexCount; i++)
		{
			s_Data.QuadVertexBufferPtr->Position = transform * s_Data.QuadVertexPositions[i];
			s_Data.QuadVertexBufferPtr->Color = color;
			s_Data.QuadVertexBufferPtr->Normals = normalMat * s_Data.QuadNormalPositions[i];
			s_Data.QuadVertexBufferPtr->TexCoords = s_Data.textureCoords[i];
			s_Data.QuadVertexBufferPtr->TextureIndex = whiteTexIndex;
			s_Data.QuadVertexBufferPtr->TilingFactor = TilingFactor;
			s_Data.QuadVertexBufferPtr->light = light;
			s_Data.QuadVertexBufferPtr++;
		}

		s_Data.QuadIndexCount += 36;

		s_Data.Stats.QuadCount++;
	}

	void Renderer3D::DrawQuad(const glm::vec3& position, const glm::vec3& scale, const Ref<Texture>& texture, float tiling, const glm::vec4& tintcolor)
	{
		PROFILE_FUNCTION();

		if (s_Data.QuadIndexCount >= RendererData::MaxIndices)
			NextBatch();

		// textureIndex is the index that will be submitted in the VBO with everything and then passed on to the fragment shader so 
		// that the shader knows which index from the sampler to sample from.

		// So here we need to find the texture index of the passed index and check if it has already been used.
		// If it the case where it has been used before, the index will be already found in the array and we just return the index
		float textureIndex = 0.0f;
		for (uint32_t i = 1; i < s_Data.TextureSlotIndex; i++)
		{
			if (*s_Data.TextureSlots[i] == *texture)
			{
				textureIndex = (float)i;
				break;
			}
		}

		// s_Data.TextureSlotIndex is the next available index in the sampler
		// If the case happens that it has never been used before, here we just add that index to the array so that it can be used later.
		if (textureIndex == 0.0f)
		{
			textureIndex = (float)s_Data.TextureSlotIndex;
			s_Data.TextureSlots[s_Data.TextureSlotIndex] = texture;
			s_Data.TextureSlotIndex++;
		}

		const int light = 0;

		glm::mat4 transform = glm::translate(glm::mat4(1.0f), position)
			* glm::scale(glm::mat4(1.0f), scale);

		glm::mat3 normalMat = glm::mat3(glm::transpose(glm::inverse(transform)));

		for (uint32_t i = 0; i < s_Data.quadVertexCount; i++)
		{
			s_Data.QuadVertexBufferPtr->Position = transform * s_Data.QuadVertexPositions[i];
			s_Data.QuadVertexBufferPtr->Color = tintcolor;
			s_Data.QuadVertexBufferPtr->Normals = normalMat * s_Data.QuadNormalPositions[i];
			s_Data.QuadVertexBufferPtr->TexCoords = s_Data.textureCoords[i];
			s_Data.QuadVertexBufferPtr->TextureIndex = textureIndex;
			s_Data.QuadVertexBufferPtr->TilingFactor = tiling;
			s_Data.QuadVertexBufferPtr->light = light;
			s_Data.QuadVertexBufferPtr++;
		}

		s_Data.QuadIndexCount += 36;

		s_Data.Stats.QuadCount++;
	}

	void Renderer3D::DrawRotatedQuad(const glm::vec3& position, const glm::vec3& rotations, const glm::vec3& scale, const glm::vec4& color, int light)// Should take a rotation!
	{
		PROFILE_FUNCTION();

		if (s_Data.QuadIndexCount >= RendererData::MaxIndices)
			NextBatch();

		const float whiteTexIndex = 0.0f; // White texture.
		const float TilingFactor = 1.0f; // TilingFactor.

		glm::mat4 transform = glm::translate(glm::mat4(1.0f), position)
			* glm::rotate(glm::mat4(1.0f), glm::radians(rotations.x), { 1.0f, 0.0f, 0.0f })
			* glm::rotate(glm::mat4(1.0f), glm::radians(rotations.y), { 0.0f, 1.0f, 0.0f })
			* glm::rotate(glm::mat4(1.0f), glm::radians(rotations.z), { 0.0f, 0.0f, 1.0f })
			* glm::scale(glm::mat4(1.0f), scale);

		glm::mat3 normalMat = glm::mat3(glm::transpose(glm::inverse(transform)));

		for (uint32_t i = 0; i < s_Data.quadVertexCount; i++)
		{
			s_Data.QuadVertexBufferPtr->Position = transform * s_Data.QuadVertexPositions[i];
			s_Data.QuadVertexBufferPtr->Color = color;
			s_Data.QuadVertexBufferPtr->Normals = normalMat * s_Data.QuadNormalPositions[i];
			s_Data.QuadVertexBufferPtr->TexCoords = s_Data.textureCoords[i];
			s_Data.QuadVertexBufferPtr->TextureIndex = whiteTexIndex;
			s_Data.QuadVertexBufferPtr->TilingFactor = TilingFactor;
			s_Data.QuadVertexBufferPtr->light = light;
			s_Data.QuadVertexBufferPtr++;
		}

		s_Data.QuadIndexCount += 36;

		s_Data.Stats.QuadCount++;
	}

	void Renderer3D::DrawRotatedQuad(const glm::vec3& position, const glm::vec3& rotations, const glm::vec3& scale, const Ref<Texture>& texture, float tiling, const glm::vec4& tintColor)
	{
		PROFILE_FUNCTION();

		if (s_Data.QuadIndexCount >= RendererData::MaxIndices)
			NextBatch();

		float textureIndex = 0.0f;
		for (uint32_t i = 1; i < s_Data.TextureSlotIndex; i++)
		{
			if (*s_Data.TextureSlots[i] == *texture)
			{
				textureIndex = (float)i;
				break;
			}
		}

		if (textureIndex == 0.0f)
		{
			textureIndex = (float)s_Data.TextureSlotIndex;
			s_Data.TextureSlots[s_Data.TextureSlotIndex] = texture;
			s_Data.TextureSlotIndex++;
		}

		const int light = 0;

		glm::mat4 transform = glm::translate(glm::mat4(1.0f), position)
			* glm::rotate(glm::mat4(1.0f), glm::radians(rotations.x), { 1.0f, 0.0f, 0.0f })
			* glm::rotate(glm::mat4(1.0f), glm::radians(rotations.y), { 0.0f, 1.0f, 0.0f })
			* glm::rotate(glm::mat4(1.0f), glm::radians(rotations.z), { 0.0f, 0.0f, 1.0f })
			* glm::scale(glm::mat4(1.0f), scale);

		glm::mat3 normalMat = glm::mat3(glm::transpose(glm::inverse(transform)));

		for (uint32_t i = 0; i < s_Data.quadVertexCount; i++)
		{
			s_Data.QuadVertexBufferPtr->Position = transform * s_Data.QuadVertexPositions[i];
			s_Data.QuadVertexBufferPtr->Color = tintColor;
			s_Data.QuadVertexBufferPtr->Normals = normalMat * s_Data.QuadNormalPositions[i];
			s_Data.QuadVertexBufferPtr->TexCoords = s_Data.textureCoords[i];
			s_Data.QuadVertexBufferPtr->TextureIndex = textureIndex;
			s_Data.QuadVertexBufferPtr->TilingFactor = tiling;
			s_Data.QuadVertexBufferPtr->light = light;
			s_Data.QuadVertexBufferPtr++;
		}

		s_Data.QuadIndexCount += 36;

		s_Data.Stats.QuadCount++;
	}

	void Renderer3D::ResetStats()
	{
		s_Data.Stats.DrawCalls = 0;
		s_Data.Stats.QuadCount = 0;
	}

	Renderer3D::Statistics& Renderer3D::GetStats()
	{
		return s_Data.Stats;
	}

}