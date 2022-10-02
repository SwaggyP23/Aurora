#include "Aurorapch.h"
#include "Renderer2D.h"

#include "Renderer.h"
#include "Core/Application.h"
#include "Graphics/UniformBuffer.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

#include <ImGui/imgui.h>
#include <glad/glad.h>

/*
 * NOTE: Setting up the Vertex Attributes are unrolled on purpose!
 */

namespace Aurora {

	Ref<Renderer2D> Renderer2D::Create()
	{
		return CreateRef<Renderer2D>();
	}

	Renderer2D::Renderer2D(const Renderer2DSpecification& spec)
		: m_Specification(spec)
	{
		Init();
	}

	Renderer2D::~Renderer2D()
	{
		ShutDown();
	}

	void Renderer2D::Init()
	{
		m_QuadVertexPositions[0] = { -0.5f, -0.5f, 0.0f, 1.0f };
		m_QuadVertexPositions[1] = { -0.5f,  0.5f, 0.0f, 1.0f };
		m_QuadVertexPositions[2] = {  0.5f,  0.5f, 0.0f, 1.0f };
		m_QuadVertexPositions[3] = {  0.5f, -0.5f, 0.0f, 1.0f };

		m_TextureCoords[0] = { 0.0f, 0.0f };
		m_TextureCoords[1] = { 1.0f, 0.0f };
		m_TextureCoords[2] = { 1.0f, 1.0f };
		m_TextureCoords[3] = { 0.0f, 1.0f };

		uint32_t* quadIndices = new uint32_t[MaxIndices];

		uint32_t offset = 0;
		for (uint32_t i = 0; i < MaxIndices; i += 6)
		{
			quadIndices[i + 0] = 0;
			quadIndices[i + 1] = 1;
			quadIndices[i + 2] = 2;

			quadIndices[i + 3] = 2;
			quadIndices[i + 4] = 3;
			quadIndices[i + 5] = 0;

			offset += 4;
		}

		m_QuadIndexBuffer = IndexBuffer::Create(quadIndices, MaxIndices);
		delete[] quadIndices;

		m_QuadVertexArray = VertexArray::Create();
		m_QuadVertexBuffer = VertexBuffer::Create(MaxVertices * sizeof(QuadVertex), BufferUsage::Dynamic);
		m_QuadVertexBuffer->SetLayout({
			{ ShaderDataType::Float3, "a_Position"    },
			{ ShaderDataType::Float4, "a_Color"       },
			{ ShaderDataType::Float2, "a_TexCoord"    },
			{ ShaderDataType::Float, "a_TexIndex"     },
			{ ShaderDataType::Float, "a_TilingFactor" }
		});
		m_QuadVertexArray->AddVertexBuffer(m_QuadVertexBuffer);
		m_QuadVertexArray->SetIndexBuffer(m_QuadIndexBuffer);

		m_QuadVertexBufferBase = new QuadVertex[MaxVertices];

		m_WhiteTexture = Renderer::GetWhiteTexture();

		// Set all the textures to a default white texture at first
		for (uint32_t i = 0; i < m_TextureSlots.size(); i++)
			m_TextureSlots[i] = m_WhiteTexture;

		// Renderer2D Exclusive camera Uniform Buffer at Binding point 2
		m_CamUniformBuffer = UniformBuffer::Create(sizeof(UBCamera), 2);

		m_QuadMaterial = Material::Create("QuadMaterial", Renderer::GetShaderLibrary()->Get("Renderer2D"));

		m_TargetRenderPass = nullptr;
	}

	void Renderer2D::ShutDown()
	{
		delete[] m_QuadVertexBufferBase;
	}

	void Renderer2D::SetTargetRenderPass(Ref<RenderPass> renderPass)
	{
		m_TargetRenderPass = renderPass;
	}

	void Renderer2D::BeginScene(const glm::mat4& viewProj, const glm::mat4& view, bool depthTest)
	{
		AR_PROFILE_FUNCTION();

		m_CameraViewProj = viewProj;
		m_CameraView = view;
		m_DepthTest = depthTest;

		// Update uniform buffer
		m_CamUniformBuffer->SetData(&viewProj, sizeof(UBCamera));

		m_QuadIndexCount = 0;
		m_QuadVertexBufferPtr = m_QuadVertexBufferBase;

		m_TextureSlotIndex = 1;

		for (uint32_t i = 0; i < m_TextureSlots.size(); i++)
			m_TextureSlots[i] = m_WhiteTexture;
	}

	void Renderer2D::FlushAndReset()
	{
		EndScene();

		m_QuadIndexCount = 0;
		m_QuadVertexBufferPtr = m_QuadVertexBufferBase;

		m_TextureSlotIndex = 1;

		for (uint32_t i = 0; i < m_TextureSlots.size(); i++)
			m_TextureSlots[i] = m_WhiteTexture;
	}

	void Renderer2D::EndScene()
	{
		AR_CORE_ASSERT(m_TargetRenderPass, "Should specify a target renderPass to render to!");

		Renderer::BeginRenderPass(m_TargetRenderPass);
		
		AR_CORE_ASSERT(m_QuadVertexBufferPtr >= m_QuadVertexBufferBase);
		uint32_t dataSize = (uint32_t)((uint8_t*)m_QuadVertexBufferPtr - (uint8_t*)m_QuadVertexBufferBase);
		if (dataSize)
		{
			m_QuadVertexBuffer->SetData(m_QuadVertexBufferBase, dataSize);

			// TODO: Bind textures for now. This SHOULD/WILL change to be handled by the material when it supports texture arrays
			for (uint32_t i = 0; i < m_TextureSlotIndex; i++)
				m_TextureSlots[i]->Bind(i);

			m_QuadMaterial->SetUpForRendering();
			m_QuadVertexArray->Bind();

			if (!m_DepthTest)
				glDisable(GL_DEPTH_TEST);

			glDisable(GL_CULL_FACE);
			//glDrawElements(GL_TRIANGLES, m_QuadIndexCount, GL_UNSIGNED_INT, nullptr);
			glDrawElementsBaseVertex(GL_TRIANGLES, m_QuadIndexCount, GL_UNSIGNED_INT, nullptr, 0);
			//Renderer::RenderGeometry(nullptr, nullptr, m_QuadMaterial, m_QuadVertexArray, m_QuadIndexCount);
			glEnable(GL_CULL_FACE);

			if (!m_DepthTest)
				glEnable(GL_DEPTH_TEST);

			m_Stats.DrawCalls++;
		}

		Renderer::EndRenderPass(m_TargetRenderPass);
	}

	void Renderer2D::DrawQuad(const glm::vec3& position, const glm::vec2& scale, const glm::vec4& color)
	{
		if (m_QuadIndexCount >= MaxIndices)
			FlushAndReset();

		constexpr float whiteTexIndex = 0.0f; // White texture.
		constexpr float tilingFactor = 1.0f; // TilingFactor.

		glm::mat4 transform = glm::translate(glm::mat4(1.0f), position)
			* glm::scale(glm::mat4(1.0f), { scale.x, scale.y, 1.0f });

		m_QuadVertexBufferPtr->Position = transform * m_QuadVertexPositions[0];
		m_QuadVertexBufferPtr->Color = color;
		m_QuadVertexBufferPtr->TexCoord = m_TextureCoords[0];
		m_QuadVertexBufferPtr->TextureIndex = whiteTexIndex;
		m_QuadVertexBufferPtr->TilingFactor = tilingFactor;
		m_QuadVertexBufferPtr++;

		m_QuadVertexBufferPtr->Position = transform * m_QuadVertexPositions[1];
		m_QuadVertexBufferPtr->Color = color;
		m_QuadVertexBufferPtr->TexCoord = m_TextureCoords[1];
		m_QuadVertexBufferPtr->TextureIndex = whiteTexIndex;
		m_QuadVertexBufferPtr->TilingFactor = tilingFactor;
		m_QuadVertexBufferPtr++;

		m_QuadVertexBufferPtr->Position = transform * m_QuadVertexPositions[2];
		m_QuadVertexBufferPtr->Color = color;
		m_QuadVertexBufferPtr->TexCoord = m_TextureCoords[2];
		m_QuadVertexBufferPtr->TextureIndex = whiteTexIndex;
		m_QuadVertexBufferPtr->TilingFactor = tilingFactor;
		m_QuadVertexBufferPtr++;

		m_QuadVertexBufferPtr->Position = transform * m_QuadVertexPositions[3];
		m_QuadVertexBufferPtr->Color = color;
		m_QuadVertexBufferPtr->TexCoord = m_TextureCoords[3];
		m_QuadVertexBufferPtr->TextureIndex = whiteTexIndex;
		m_QuadVertexBufferPtr->TilingFactor = tilingFactor;
		m_QuadVertexBufferPtr++;

		m_QuadIndexCount += 6;

		m_Stats.QuadCount++;
	}

	void Renderer2D::DrawQuad(const glm::vec3& position, const glm::vec2& scale, const Ref<Texture2D>& texture, float tiling, const glm::vec4& tintcolor)
	{
		AR_SCOPE_PERF("Renderer2D::DrawQuad");

		if (m_QuadIndexCount >= MaxIndices)
			FlushAndReset();

		// textureIndex is the index that will be submitted in the VBO with everything and then passed on to the fragment shader so 
		// that the shader knows which index from the sampler to sample from.

		// So here we need to find the texture index of the passed index and check if it has already been used.
		// If it the case where it has been used before, the index will be already found in the array and we just return the index
		float textureIndex = 0.0f;
		for (uint32_t i = 1; i < m_TextureSlotIndex; i++)
		{
			if (m_TextureSlots[i] == texture)
			{
				textureIndex = (float)i;
				break;
			}
		}

		// m_TextureSlotIndex is the next available index in the sampler
		// If the case happens that it has never been used before, here we just add that index to the array so that it can be used later.
		if (textureIndex == 0.0f)
		{
			textureIndex = (float)m_TextureSlotIndex;
			m_TextureSlots[m_TextureSlotIndex] = texture;
			m_TextureSlotIndex++;
		}

		glm::mat4 transform = glm::translate(glm::mat4(1.0f), position)
			* glm::scale(glm::mat4(1.0f), { scale.x, scale.y, 1.0f });

		m_QuadVertexBufferPtr->Position = transform * m_QuadVertexPositions[0];
		m_QuadVertexBufferPtr->Color = tintcolor;
		m_QuadVertexBufferPtr->TexCoord = m_TextureCoords[0];
		m_QuadVertexBufferPtr->TextureIndex = textureIndex;
		m_QuadVertexBufferPtr->TilingFactor = tiling;
		m_QuadVertexBufferPtr++;

		m_QuadVertexBufferPtr->Position = transform * m_QuadVertexPositions[1];
		m_QuadVertexBufferPtr->Color = tintcolor;
		m_QuadVertexBufferPtr->TexCoord = m_TextureCoords[1];
		m_QuadVertexBufferPtr->TextureIndex = textureIndex;
		m_QuadVertexBufferPtr->TilingFactor = tiling;
		m_QuadVertexBufferPtr++;

		m_QuadVertexBufferPtr->Position = transform * m_QuadVertexPositions[2];
		m_QuadVertexBufferPtr->Color = tintcolor;
		m_QuadVertexBufferPtr->TexCoord = m_TextureCoords[2];
		m_QuadVertexBufferPtr->TextureIndex = textureIndex;
		m_QuadVertexBufferPtr->TilingFactor = tiling;
		m_QuadVertexBufferPtr++;

		m_QuadVertexBufferPtr->Position = transform * m_QuadVertexPositions[3];
		m_QuadVertexBufferPtr->Color = tintcolor;
		m_QuadVertexBufferPtr->TexCoord = m_TextureCoords[3];
		m_QuadVertexBufferPtr->TextureIndex = textureIndex;
		m_QuadVertexBufferPtr->TilingFactor = tiling;
		m_QuadVertexBufferPtr++;

		m_QuadIndexCount += 6;

		m_Stats.QuadCount++;
	}

	void Renderer2D::DrawRotatedQuad(const glm::vec3& position, const glm::vec3& rotations, const glm::vec2& scale, const glm::vec4& color)
	{
		AR_SCOPE_PERF("Renderer2D::DrawRotatedQuad");

		if (m_QuadIndexCount >= MaxIndices)
			FlushAndReset();

		constexpr float TilingFactor = 1.0f; // TilingFactor.
		constexpr float whiteTexIndex = 0.0f; // White texture.

		glm::mat4 Rotation = glm::toMat4(glm::quat(rotations));
		glm::mat4 transform = glm::translate(glm::mat4(1.0f), position) * Rotation * glm::scale(glm::mat4(1.0f), { scale.x, scale.y, 1.0f });

		m_QuadVertexBufferPtr->Position = transform * m_QuadVertexPositions[0];
		m_QuadVertexBufferPtr->Color = color;
		m_QuadVertexBufferPtr->TexCoord = m_TextureCoords[0];
		m_QuadVertexBufferPtr->TextureIndex = whiteTexIndex;
		m_QuadVertexBufferPtr->TilingFactor = TilingFactor;
		m_QuadVertexBufferPtr++;

		m_QuadVertexBufferPtr->Position = transform * m_QuadVertexPositions[1];
		m_QuadVertexBufferPtr->Color = color;
		m_QuadVertexBufferPtr->TexCoord = m_TextureCoords[1];
		m_QuadVertexBufferPtr->TextureIndex = whiteTexIndex;
		m_QuadVertexBufferPtr->TilingFactor = TilingFactor;
		m_QuadVertexBufferPtr++;

		m_QuadVertexBufferPtr->Position = transform * m_QuadVertexPositions[2];
		m_QuadVertexBufferPtr->Color = color;
		m_QuadVertexBufferPtr->TexCoord = m_TextureCoords[2];
		m_QuadVertexBufferPtr->TextureIndex = whiteTexIndex;
		m_QuadVertexBufferPtr->TilingFactor = TilingFactor;
		m_QuadVertexBufferPtr++;

		m_QuadVertexBufferPtr->Position = transform * m_QuadVertexPositions[3];
		m_QuadVertexBufferPtr->Color = color;
		m_QuadVertexBufferPtr->TexCoord = m_TextureCoords[3];
		m_QuadVertexBufferPtr->TextureIndex = whiteTexIndex;
		m_QuadVertexBufferPtr->TilingFactor = TilingFactor;
		m_QuadVertexBufferPtr++;

		m_QuadIndexCount += 6;

		m_Stats.QuadCount++;
	}

	void Renderer2D::DrawRotatedQuad(const glm::vec3& position, const glm::vec3& rotations, const glm::vec2& scale, const Ref<Texture2D>& texture, float tiling, const glm::vec4& tintColor)
	{
		AR_SCOPE_PERF("Renderer2D::DrawRotatedQuad");

		if (m_QuadIndexCount >= MaxIndices)
			FlushAndReset();

		// textureIndex is the index that will be submitted in the VBO with everything and then passed on to the fragment shader so 
		// that the shader knows which index from the sampler to sample from.
		
		// So here we need to find the texture index of the passed index and check if it has already been used.
		// If it the case where it has been used before, the index will be already found in the array and we just return the index
		float textureIndex = 0.0f;
		for (uint32_t i = 1; i < m_TextureSlotIndex; i++)
		{
			if (m_TextureSlots[i] == texture)
			{
				textureIndex = (float)i;
				break;
			}
		}

		// m_TextureSlotIndex is the next available index in the sampler
		// If the case happens that it has never been used before, here we just add that index to the array so that it can be used later.
		if (textureIndex == 0.0f)
		{
			textureIndex = (float)m_TextureSlotIndex;
			m_TextureSlots[m_TextureSlotIndex] = texture;
			m_TextureSlotIndex++;
		}

		glm::mat4 Rotation = glm::toMat4(glm::quat(rotations));
		glm::mat4 transform = glm::translate(glm::mat4(1.0f), position) * Rotation * glm::scale(glm::mat4(1.0f), { scale.x, scale.y, 1.0f });

		m_QuadVertexBufferPtr->Position = transform * m_QuadVertexPositions[0];
		m_QuadVertexBufferPtr->Color = tintColor;
		m_QuadVertexBufferPtr->TexCoord = m_TextureCoords[0];
		m_QuadVertexBufferPtr->TextureIndex = textureIndex;
		m_QuadVertexBufferPtr->TilingFactor = tiling;
		m_QuadVertexBufferPtr++;

		m_QuadVertexBufferPtr->Position = transform * m_QuadVertexPositions[1];
		m_QuadVertexBufferPtr->Color = tintColor;
		m_QuadVertexBufferPtr->TexCoord = m_TextureCoords[1];
		m_QuadVertexBufferPtr->TextureIndex = textureIndex;
		m_QuadVertexBufferPtr->TilingFactor = tiling;
		m_QuadVertexBufferPtr++;

		m_QuadVertexBufferPtr->Position = transform * m_QuadVertexPositions[2];
		m_QuadVertexBufferPtr->Color = tintColor;
		m_QuadVertexBufferPtr->TexCoord = m_TextureCoords[2];
		m_QuadVertexBufferPtr->TextureIndex = textureIndex;
		m_QuadVertexBufferPtr->TilingFactor = tiling;
		m_QuadVertexBufferPtr++;

		m_QuadVertexBufferPtr->Position = transform * m_QuadVertexPositions[3];
		m_QuadVertexBufferPtr->Color = tintColor;
		m_QuadVertexBufferPtr->TexCoord = m_TextureCoords[3];
		m_QuadVertexBufferPtr->TextureIndex = textureIndex;
		m_QuadVertexBufferPtr->TilingFactor = tiling;
		m_QuadVertexBufferPtr++;

		m_QuadIndexCount += 6;

		m_Stats.QuadCount++;
	}

	void Renderer2D::SetLineWidth(float lineWidth)
	{
		m_LineWidth = lineWidth;
	}

	void Renderer2D::ResetStats()
	{
		m_Stats.DrawCalls = 0;
		m_Stats.QuadCount = 0;
	}

	const Renderer2D::Statistics& Renderer2D::GetStats()
	{
		return m_Stats;
	}

}