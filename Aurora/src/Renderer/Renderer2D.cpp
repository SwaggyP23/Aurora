#include "Aurorapch.h"
#include "Renderer2D.h"

#include "Renderer.h"
#include "Core/Application.h"
#include "Graphics/MSDFData.h"
#include "Graphics/UniformBuffer.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

#include <ImGui/imgui.h>
#include <glad/glad.h>

// TODO: May be temporary untill i find a better way to convert to utf32
#include <codecvt>

/*
 * NOTE: Setting up the Vertex Attributes are unrolled on purpose!
 */

 // Define to 1 if you want to visualize what the Renderer2D is rendering by making it render to its own framebuffer!
#define RENDERER2D_DEBUG 0

namespace Aurora {

	namespace Utils {

// warning C4996: 'std::codecvt_utf8<char32_t,1114111,(std::codecvt_mode)0>': warning STL4017: std::wbuffer_convert, std::wstring_convert, and the <codecvt> header
// (containing std::codecvt_mode, std::codecvt_utf8, std::codecvt_utf16, and std::codecvt_utf8_utf16) are deprecated in C++17. (The std::codecvt class template is NOT deprecated.)
// The C++ Standard doesn't provide equivalent non-deprecated functionality; consider using MultiByteToWideChar() and WideCharToMultiByte() from <Windows.h> instead.
// You can define _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING or _SILENCE_ALL_CXX17_DEPRECATION_WARNINGS to acknowledge that you have received this warning.
#pragma warning(disable : 4996)

		static std::u32string To_UTF32(const std::string& s)
		{
			// From https://stackoverflow.com/questions/31302506/stdu32string-conversion-to-from-stdstring-and-stdu16string
			std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> conv;

			return conv.from_bytes(s);
		}

#pragma warning(default: 4996)

		static bool NextLine(int index, const std::vector<int>& lines)
		{
			for (int line : lines)
			{
				if (line == index)
					return true;
			}

			return false;
		}

	}

	Ref<Renderer2D> Renderer2D::Create(const Renderer2DSpecification& spec)
	{
		return CreateRef<Renderer2D>(spec);
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
#if RENDERER2D_DEBUG
		FramebufferSpecification spec = {};
		spec.DebugName = "Renderer2D_DebugFBO";
		spec.AttachmentsSpecification = { ImageFormat::RGBA, ImageFormat::Depth };
		spec.ClearColor = { 0.05f, 0.4f, 0.1f, 1.0f };
		//spec.ClearOnBind = false;
		spec.Width = 1280;
		spec.Height = 720;

		RenderPassSpecification rspec = {};
		rspec.DebugName = "Renderer2DDebugPass";
		rspec.TargetFramebuffer = Framebuffer::Create(spec);
		m_DebugRenderPass = RenderPass::Create(rspec);
#endif

		uint32_t* quadIndices = new uint32_t[MaxQuadIndices];

		uint32_t offset = 0;
		for (uint32_t i = 0; i < MaxQuadIndices; i += 6)
		{
			quadIndices[i + 0] = offset + 0;
			quadIndices[i + 1] = offset + 1;
			quadIndices[i + 2] = offset + 2;

			quadIndices[i + 3] = offset + 2;
			quadIndices[i + 4] = offset + 3;
			quadIndices[i + 5] = offset + 0;

			offset += 4;
		}

		Ref<IndexBuffer> quadIndexBuffer = IndexBuffer::Create(quadIndices, MaxQuadIndices);
		delete[] quadIndices;

		// Quads...
		{
			m_QuadVertexArray = VertexArray::Create();

			m_QuadVertexBuffer = VertexBuffer::Create(MaxQuadVertices * sizeof(QuadVertex));
			m_QuadVertexBuffer->SetLayout({
				{ ShaderDataType::Float3, "a_Position"     },
				{ ShaderDataType::Float4, "a_Color"        },
				{ ShaderDataType::Float2, "a_TexCoord"     },
				{ ShaderDataType::Float,  "a_TexIndex"     },
				{ ShaderDataType::Float,  "a_TilingFactor" }
				});
			m_QuadVertexArray->AddVertexBuffer(m_QuadVertexBuffer);
			m_QuadVertexArray->SetIndexBuffer(quadIndexBuffer);

			m_QuadVertexBufferBase = new QuadVertex[MaxQuadVertices];

			m_QuadMaterial = Material::Create("Renderer2D_Quad", Renderer::GetShaderLibrary()->Get("Renderer2DQuad"));
		}

		// Lines...
		{
			uint32_t* lineIndices = new uint32_t[MaxLineIndices];

			for (uint32_t i = 0; i < MaxLineIndices; i++)
				lineIndices[i] = i;

			Ref<IndexBuffer> lineIndexBuffer = IndexBuffer::Create(lineIndices, MaxLineIndices);
			delete[] lineIndices;

			m_LineVertexArray = VertexArray::Create();

			m_LineVertexBuffer = VertexBuffer::Create(MaxLineVertices * sizeof(LineVertex));
			m_LineVertexBuffer->SetLayout({
				{ ShaderDataType::Float3, "a_Position" },
				{ ShaderDataType::Float4, "a_Color"    }
				});
			m_LineVertexArray->AddVertexBuffer(m_LineVertexBuffer);
			m_LineVertexArray->SetIndexBuffer(lineIndexBuffer);

			m_LineVertexBufferBase = new LineVertex[MaxLineVertices];

			m_LineMaterial = Material::Create("Renderer2D_Line", Renderer::GetShaderLibrary()->Get("Renderer2DLine"));
			//m_LineMaterial->SetFlag(MaterialFlag::DepthTest, false); // TODO: Make this optional...
		}

		// Circles...
		{
			m_CircleVertexArray = VertexArray::Create();

			m_CircleVertexBuffer = VertexBuffer::Create(MaxQuadVertices * sizeof(CircleVertex));
			m_CircleVertexBuffer->SetLayout({
				{ ShaderDataType::Float3, "a_WorldPosition" },
				{ ShaderDataType::Float,  "a_Thickness"     },
				{ ShaderDataType::Float2, "a_LocalPosition" },
				{ ShaderDataType::Float4, "a_Color"         }
			});
			m_CircleVertexArray->AddVertexBuffer(m_CircleVertexBuffer);
			m_CircleVertexArray->SetIndexBuffer(quadIndexBuffer);

			m_CircleVertexBufferBase = new CircleVertex[MaxQuadVertices];

			m_CircleMaterial = Material::Create("Renderer2D_Circle", Renderer::GetShaderLibrary()->Get("Renderer2DCircle"));
		}

		// Text...
		{
			m_TextVertexArray = VertexArray::Create();

			m_TextVertexBuffer = VertexBuffer::Create(MaxQuadVertices * sizeof(TextVertex));
			m_TextVertexBuffer->SetLayout({
				{ ShaderDataType::Float3, "a_Position" },
				{ ShaderDataType::Float4, "a_Color"    },
				{ ShaderDataType::Float2, "a_TexCoord" },
				{ ShaderDataType::Float, "a_TexIndex"  }
			});
			m_TextVertexArray->AddVertexBuffer(m_TextVertexBuffer);
			m_TextVertexArray->SetIndexBuffer(quadIndexBuffer);

			m_TextVertexBufferBase = new TextVertex[MaxQuadVertices];

			m_TextMaterial = Material::Create("Renderer2D_Text", Renderer::GetShaderLibrary()->Get("Renderer2DText"));
			m_TextMaterial->SetFlag(MaterialFlag::TwoSided, true);
		}

		m_WhiteTexture = Renderer::GetWhiteTexture();

		// Set first texture slot to the white texture
		m_TextureSlots[0] = m_WhiteTexture;

		m_QuadVertexPositions[0] = { -0.5f, -0.5f, 0.0f, 1.0f };
		m_QuadVertexPositions[1] = { 0.5f, -0.5f, 0.0f, 1.0f };
		m_QuadVertexPositions[2] = { 0.5f,  0.5f, 0.0f, 1.0f };
		m_QuadVertexPositions[3] = { -0.5f,  0.5f, 0.0f, 1.0f };

		m_TextureCoords[0] = { 0.0f, 0.0f };
		m_TextureCoords[1] = { 1.0f, 0.0f };
		m_TextureCoords[2] = { 1.0f, 1.0f };
		m_TextureCoords[3] = { 0.0f, 1.0f };

		m_TargetRenderPass = nullptr;

		m_CamUniformBuffer = UniformBuffer::Create(sizeof(UBCamera), 3);
	}

	void Renderer2D::ShutDown()
	{
		delete[] m_QuadVertexBufferBase;

		delete[] m_LineVertexBufferBase;

		delete[] m_CircleVertexBufferBase;

		delete[] m_TextVertexBufferBase;
	}

	void Renderer2D::SetTargetRenderPass(Ref<RenderPass> renderPass)
	{
		m_TargetRenderPass = renderPass;
	}

	Ref<Texture2D> Renderer2D::GetDebugFinalImage()
	{
		return m_DebugRenderPass->GetSpecification().TargetFramebuffer->GetColorAttachment(0);
	}

	Ref<RenderPass> Renderer2D::GetDebugRenderPass()
	{
		return m_DebugRenderPass;
	}

	void Renderer2D::BeginScene(const glm::mat4& viewProj, const glm::mat4& view, bool depthTest)
	{
		AR_PROFILE_FUNCTION();

		m_CameraViewProj = viewProj;
		m_CameraView = view;
		m_DepthTest = depthTest;

		// Update uniform buffer
		m_CamUniformBuffer->SetData(&m_CameraViewProj, sizeof(UBCamera));

		m_QuadIndexCount = 0;
		m_QuadVertexBufferPtr = m_QuadVertexBufferBase;

		m_LineIndexCount = 0;
		m_LineVertexBufferPtr = m_LineVertexBufferBase;

		m_CircleIndexCount = 0;
		m_CircleVertexBufferPtr = m_CircleVertexBufferBase;

		m_TextIndexCount = 0;
		m_TextVertexBufferPtr = m_TextVertexBufferBase;

		m_TextureSlotIndex = 1;
		m_FontTextureSlotIndex = 0;
	}

	void Renderer2D::FlushAndReset()
	{
		EndScene();

		m_QuadIndexCount = 0;
		m_QuadVertexBufferPtr = m_QuadVertexBufferBase;

		m_LineIndexCount = 0;
		m_LineVertexBufferPtr = m_LineVertexBufferBase;

		m_CircleIndexCount = 0;
		m_CircleVertexBufferPtr = m_CircleVertexBufferBase;

		m_TextIndexCount = 0;
		m_TextVertexBufferPtr = m_TextVertexBufferBase;

		m_TextureSlotIndex = 1;
		m_FontTextureSlotIndex = 0;
	}

	void Renderer2D::EndScene()
	{
		AR_CORE_ASSERT(m_TargetRenderPass, "Should specify a target renderPass to render to!");

#if RENDERER2D_DEBUG
		Renderer::BeginRenderPass(m_DebugRenderPass);
#else
		Renderer::BeginRenderPass(m_TargetRenderPass);
#endif
		
		// Disable culling for the whole 2D Pass since we dont want everything to be visible from one side only!
		glDisable(GL_CULL_FACE);

		// Quads...
		AR_CORE_ASSERT(m_QuadVertexBufferPtr >= m_QuadVertexBufferBase);
		uint32_t dataSize = (uint32_t)((uint8_t*)m_QuadVertexBufferPtr - (uint8_t*)m_QuadVertexBufferBase);
		if (dataSize)
		{
			m_QuadVertexBuffer->SetData(m_QuadVertexBufferBase, dataSize);

			// TODO: Bind textures for now. This SHOULD/WILL change to be handled by the material when it supports texture arrays
			for (uint32_t i = 0; i < m_TextureSlotIndex; i++)
				m_TextureSlots[i]->Bind(i);

			if (!m_DepthTest)
				glDisable(GL_DEPTH_TEST);

			Renderer::RenderGeometry(nullptr, nullptr, m_QuadMaterial, m_QuadVertexArray, m_QuadIndexCount);

			if (!m_DepthTest)
				glEnable(GL_DEPTH_TEST);

			m_Stats.DrawCalls++;
		}

		// Lines...
		AR_CORE_ASSERT(m_LineVertexBufferPtr >= m_LineVertexBufferBase);
		dataSize = (uint32_t)((uint8_t*)m_LineVertexBufferPtr - (uint8_t*)m_LineVertexBufferBase);
		if (dataSize)
		{
			m_LineVertexBuffer->SetData(m_LineVertexBufferBase, dataSize);

			glLineWidth(m_LineWidth);

			if (!m_LineMaterial->HasFlag(MaterialFlag::DepthTest))
				glDisable(GL_DEPTH_TEST);

			m_LineMaterial->SetUpForRendering();
			m_LineVertexArray->Bind();
			glDrawElementsBaseVertex(GL_LINES, m_LineIndexCount, GL_UNSIGNED_INT, nullptr, 0);

			if (!m_LineMaterial->HasFlag(MaterialFlag::DepthTest))
				glEnable(GL_DEPTH_TEST);

			m_Stats.DrawCalls++;
		}

		// Text...
		AR_CORE_ASSERT(m_TextVertexBufferPtr >= m_TextVertexBufferBase);
		dataSize = (uint32_t)((uint8_t*)m_TextVertexBufferPtr - (uint8_t*)m_TextVertexBufferBase);
		if (dataSize)
		{
			m_TextVertexBuffer->SetData(m_TextVertexBufferBase, dataSize);

			// TODO: Bind textures for now. This SHOULD/WILL change to be handled by the material when it supports texture arrays
			for (uint32_t i = 0; i < m_FontTextureSlotIndex; i++)
				m_FontTextureSlots[i]->Bind(i);

			Renderer::RenderGeometry(nullptr, nullptr, m_TextMaterial, m_TextVertexArray, m_TextIndexCount);

			m_Stats.DrawCalls++;
		}

		// Circles...
		AR_CORE_ASSERT(m_CircleVertexBufferPtr >= m_CircleVertexBufferBase);
		dataSize = (uint32_t)((uint8_t*)m_CircleVertexBufferPtr - (uint8_t*)m_CircleVertexBufferBase);
		if (dataSize)
		{
			m_CircleVertexBuffer->SetData(m_CircleVertexBufferBase, dataSize);

			Renderer::RenderGeometry(nullptr, nullptr, m_CircleMaterial, m_CircleVertexArray, m_CircleIndexCount);

			m_Stats.DrawCalls++;
		}

		// Enable culling at the end of the 2D Pass so that everything is reset back to normal
		glEnable(GL_CULL_FACE);

#if RENDERER2D_DEBUG
		Renderer::EndRenderPass(m_DebugRenderPass);
#else
		Renderer::EndRenderPass(m_TargetRenderPass);
#endif
	}

	void Renderer2D::DrawQuad(const glm::vec3& position, const glm::vec2& scale, const glm::vec4& color)
	{
		if (m_QuadIndexCount >= MaxQuadIndices)
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
		if (m_QuadIndexCount >= MaxQuadIndices)
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
			if (m_TextureSlotIndex >= MaxQuadTextureSlots)
				FlushAndReset();

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
		if (m_QuadIndexCount >= MaxQuadIndices)
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
		if (m_QuadIndexCount >= MaxQuadIndices)
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
			if (m_TextureSlotIndex >= MaxQuadTextureSlots)
				FlushAndReset();

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

	void Renderer2D::DrawQuadBillboard(const glm::vec3& position, const glm::vec2& scale, const glm::vec4& color)
	{
		if (m_QuadIndexCount >= MaxQuadIndices)
			FlushAndReset();

		constexpr float whiteTexIndex = 0.0f; // White texture.
		constexpr float tilingFactor = 1.0f; // TilingFactor.

		glm::vec3 camRightWS = { m_CameraView[0][0], m_CameraView[1][0], m_CameraView[2][0] };
		glm::vec3 camUpWS = { m_CameraView[0][1], m_CameraView[1][1], m_CameraView[2][1] };

		m_QuadVertexBufferPtr->Position = position + camRightWS * m_QuadVertexPositions[0].x * scale.x + camUpWS * m_QuadVertexPositions[0].y * scale.y;
		m_QuadVertexBufferPtr->Color = color;
		m_QuadVertexBufferPtr->TexCoord = m_TextureCoords[0];
		m_QuadVertexBufferPtr->TextureIndex = whiteTexIndex;
		m_QuadVertexBufferPtr->TilingFactor = tilingFactor;
		m_QuadVertexBufferPtr++;

		m_QuadVertexBufferPtr->Position = position + camRightWS * m_QuadVertexPositions[1].x * scale.x + camUpWS * m_QuadVertexPositions[1].y * scale.y;
		m_QuadVertexBufferPtr->Color = color;
		m_QuadVertexBufferPtr->TexCoord = m_TextureCoords[1];
		m_QuadVertexBufferPtr->TextureIndex = whiteTexIndex;
		m_QuadVertexBufferPtr->TilingFactor = tilingFactor;
		m_QuadVertexBufferPtr++;

		m_QuadVertexBufferPtr->Position = position + camRightWS * m_QuadVertexPositions[2].x * scale.x + camUpWS * m_QuadVertexPositions[2].y * scale.y;
		m_QuadVertexBufferPtr->Color = color;
		m_QuadVertexBufferPtr->TexCoord = m_TextureCoords[2];
		m_QuadVertexBufferPtr->TextureIndex = whiteTexIndex;
		m_QuadVertexBufferPtr->TilingFactor = tilingFactor;
		m_QuadVertexBufferPtr++;

		m_QuadVertexBufferPtr->Position = position + camRightWS * m_QuadVertexPositions[3].x * scale.x + camUpWS * m_QuadVertexPositions[3].y * scale.y;
		m_QuadVertexBufferPtr->Color = color;
		m_QuadVertexBufferPtr->TexCoord = m_TextureCoords[3];
		m_QuadVertexBufferPtr->TextureIndex = whiteTexIndex;
		m_QuadVertexBufferPtr->TilingFactor = tilingFactor;
		m_QuadVertexBufferPtr++;

		m_QuadIndexCount += 6;

		m_Stats.QuadCount++;
	}

	void Renderer2D::DrawQuadBillboard(const glm::vec3& position, const glm::vec2& scale, const Ref<Texture2D>& texture, float tiling, const glm::vec4& tintColor)
	{
		if (m_QuadIndexCount >= MaxQuadIndices)
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
			if (m_TextureSlotIndex >= MaxQuadTextureSlots)
				FlushAndReset();

			textureIndex = (float)m_TextureSlotIndex;
			m_TextureSlots[m_TextureSlotIndex] = texture;
			m_TextureSlotIndex++;
		}

		glm::vec3 camRightWS = { m_CameraView[0][0], m_CameraView[1][0], m_CameraView[2][0] };
		glm::vec3 camUpWS = { m_CameraView[0][1], m_CameraView[1][1], m_CameraView[2][1] };

		m_QuadVertexBufferPtr->Position = position + camRightWS * m_QuadVertexPositions[0].x * scale.x + camUpWS * m_QuadVertexPositions[0].y * scale.y;
		m_QuadVertexBufferPtr->Color = tintColor;
		m_QuadVertexBufferPtr->TexCoord = m_TextureCoords[0];
		m_QuadVertexBufferPtr->TextureIndex = textureIndex;
		m_QuadVertexBufferPtr->TilingFactor = tiling;
		m_QuadVertexBufferPtr++;

		m_QuadVertexBufferPtr->Position = position + camRightWS * m_QuadVertexPositions[1].x * scale.x + camUpWS * m_QuadVertexPositions[1].y * scale.y;
		m_QuadVertexBufferPtr->Color = tintColor;
		m_QuadVertexBufferPtr->TexCoord = m_TextureCoords[1];
		m_QuadVertexBufferPtr->TextureIndex = textureIndex;
		m_QuadVertexBufferPtr->TilingFactor = tiling;
		m_QuadVertexBufferPtr++;

		m_QuadVertexBufferPtr->Position = position + camRightWS * m_QuadVertexPositions[2].x * scale.x + camUpWS * m_QuadVertexPositions[2].y * scale.y;
		m_QuadVertexBufferPtr->Color = tintColor;
		m_QuadVertexBufferPtr->TexCoord = m_TextureCoords[2];
		m_QuadVertexBufferPtr->TextureIndex = textureIndex;
		m_QuadVertexBufferPtr->TilingFactor = tiling;
		m_QuadVertexBufferPtr++;

		m_QuadVertexBufferPtr->Position = position + camRightWS * m_QuadVertexPositions[3].x * scale.x + camUpWS * m_QuadVertexPositions[3].y * scale.y;
		m_QuadVertexBufferPtr->Color = tintColor;
		m_QuadVertexBufferPtr->TexCoord = m_TextureCoords[3];
		m_QuadVertexBufferPtr->TextureIndex = textureIndex;
		m_QuadVertexBufferPtr->TilingFactor = tiling;
		m_QuadVertexBufferPtr++;

		m_QuadIndexCount += 6;

		m_Stats.QuadCount++;
	}

	void Renderer2D::DrawCircle(const glm::vec3& position, const glm::vec3& rotation, float radius, const glm::vec4& color)
	{
		glm::mat4 Rotation = glm::toMat4(glm::quat(rotation));

		const glm::mat4 transform = glm::translate(glm::mat4(1.0f), position) * Rotation * glm::scale(glm::mat4(1.0f), glm::vec3(radius));

		DrawCircle(transform, color);
	}

	void Renderer2D::DrawCircle(const glm::mat4& transform, const glm::vec4& color)
	{
		uint32_t segments = 32;
		for (uint32_t i = 0; i < segments; i++)
		{
			float angle = 2.0f * glm::pi<float>() * (float)i / (float)segments;
			glm::vec4 startPosition = { glm::cos(angle), glm::sin(angle), 0.0f, 1.0f };
			glm::vec3 p0 = transform * startPosition;

			angle = 2.0f * glm::pi<float>() * (float)((i + 1) % segments) / (float)segments;
			glm::vec4 endPosition = { glm::cos(angle), glm::sin(angle), 0.0f, 1.0f };
			glm::vec3 p1 = transform * endPosition;

			DrawLine(p0, p1, color);
		}
	}

	// This allows for non uniform scaling of the circle
	void Renderer2D::FillCircle(const glm::mat4& transform, const glm::vec4& color, float thickness)
	{
		if (m_CircleIndexCount >= MaxQuadIndices)
			FlushAndReset();

		m_CircleVertexBufferPtr->WorldPosition = transform * m_QuadVertexPositions[0];
		m_CircleVertexBufferPtr->Thickness = thickness;
		m_CircleVertexBufferPtr->LocalPosition = m_QuadVertexPositions[0] * 2.0f;
		m_CircleVertexBufferPtr->Color = color;
		m_CircleVertexBufferPtr++;

		m_CircleVertexBufferPtr->WorldPosition = transform * m_QuadVertexPositions[1];
		m_CircleVertexBufferPtr->Thickness = thickness;
		m_CircleVertexBufferPtr->LocalPosition = m_QuadVertexPositions[1] * 2.0f;
		m_CircleVertexBufferPtr->Color = color;
		m_CircleVertexBufferPtr++;

		m_CircleVertexBufferPtr->WorldPosition = transform * m_QuadVertexPositions[2];
		m_CircleVertexBufferPtr->Thickness = thickness;
		m_CircleVertexBufferPtr->LocalPosition = m_QuadVertexPositions[2] * 2.0f;
		m_CircleVertexBufferPtr->Color = color;
		m_CircleVertexBufferPtr++;

		m_CircleVertexBufferPtr->WorldPosition = transform * m_QuadVertexPositions[3];
		m_CircleVertexBufferPtr->Thickness = thickness;
		m_CircleVertexBufferPtr->LocalPosition = m_QuadVertexPositions[3] * 2.0f;
		m_CircleVertexBufferPtr->Color = color;
		m_CircleVertexBufferPtr++;

		m_CircleIndexCount += 6;

		m_Stats.QuadCount++;
	}

	void Renderer2D::FillCircle(const glm::vec2& position, float radius, const glm::vec4& color, float thickness)
	{
		FillCircle({ position.x, position.y, 0.0f }, radius, color, thickness);
	}

	void Renderer2D::FillCircle(const glm::vec3& position, float radius, const glm::vec4& color, float thickness)
	{
		if (m_CircleIndexCount >= MaxQuadIndices)
			FlushAndReset();

		glm::mat4 transform = glm::translate(glm::mat4(1.0f), position) * glm::scale(glm::mat4(1.0f), { radius * 2.0f, radius * 2.0f, 1.0f });

		m_CircleVertexBufferPtr->WorldPosition = transform * m_QuadVertexPositions[0];
		m_CircleVertexBufferPtr->Thickness = thickness;
		m_CircleVertexBufferPtr->LocalPosition = m_QuadVertexPositions[0] * 2.0f;
		m_CircleVertexBufferPtr->Color = color;
		m_CircleVertexBufferPtr++;

		m_CircleVertexBufferPtr->WorldPosition = transform * m_QuadVertexPositions[1];
		m_CircleVertexBufferPtr->Thickness = thickness;
		m_CircleVertexBufferPtr->LocalPosition = m_QuadVertexPositions[1] * 2.0f;
		m_CircleVertexBufferPtr->Color = color;
		m_CircleVertexBufferPtr++;

		m_CircleVertexBufferPtr->WorldPosition = transform * m_QuadVertexPositions[2];
		m_CircleVertexBufferPtr->Thickness = thickness;
		m_CircleVertexBufferPtr->LocalPosition = m_QuadVertexPositions[2] * 2.0f;
		m_CircleVertexBufferPtr->Color = color;
		m_CircleVertexBufferPtr++;

		m_CircleVertexBufferPtr->WorldPosition = transform * m_QuadVertexPositions[3];
		m_CircleVertexBufferPtr->Thickness = thickness;
		m_CircleVertexBufferPtr->LocalPosition = m_QuadVertexPositions[3] * 2.0f;
		m_CircleVertexBufferPtr->Color = color;
		m_CircleVertexBufferPtr++;

		m_CircleIndexCount += 6;

		m_Stats.QuadCount++;
	}

	void Renderer2D::DrawLine(const glm::vec3& p0, const glm::vec3& p1, const glm::vec4& color)
	{
		if (m_LineIndexCount >= MaxLineIndices)
			FlushAndReset();

		m_LineVertexBufferPtr->Position = p0;
		m_LineVertexBufferPtr->Color = color;
		m_LineVertexBufferPtr++;

		m_LineVertexBufferPtr->Position = p1;
		m_LineVertexBufferPtr->Color = color;
		m_LineVertexBufferPtr++;

		m_LineIndexCount += 2;

		m_Stats.LineCount++;
	}

	void Renderer2D::DrawRotatedRect(const glm::vec3& position, const glm::vec3& rotations, const glm::vec2& scale, const glm::vec4& color)
	{
		if (m_LineIndexCount >= MaxLineIndices)
			FlushAndReset();

		glm::mat4 Rotation = glm::toMat4(glm::quat(rotations));
		glm::mat4 transform = glm::translate(glm::mat4(1.0f), position) * Rotation * glm::scale(glm::mat4(1.0f), { scale.x, scale.y, 1.0f });

		glm::vec3 positions[4] =
		{
			transform * m_QuadVertexPositions[0],
			transform * m_QuadVertexPositions[1],
			transform * m_QuadVertexPositions[2],
			transform * m_QuadVertexPositions[3]
		};

		for (uint32_t i = 0; i < 4; i++)
		{
			const glm::vec3& v0 = positions[i];
			const glm::vec3& v1 = positions[(i + 1) % 4]; // wrap the array as in wrap the rectangle's vertices

			m_LineVertexBufferPtr->Position = v0;
			m_LineVertexBufferPtr->Color = color;
			m_LineVertexBufferPtr++;

			m_LineVertexBufferPtr->Position = v1;
			m_LineVertexBufferPtr->Color = color;
			m_LineVertexBufferPtr++;

			m_LineIndexCount += 2;

			m_Stats.LineCount++;
		}
	}

	void Renderer2D::DrawAABB(Ref<StaticMesh> mesh, const glm::mat4& transform, const glm::vec4& color)
	{
		const std::vector<SubMesh>& meshSourceSubmeshes = mesh->GetMeshSource()->GetSubMeshes();
		const std::vector<uint32_t>& submeshes = mesh->GetSubMeshes();

		for (uint32_t subMeshIndex : submeshes)
		{
			const SubMesh& submesh = meshSourceSubmeshes[subMeshIndex];
			const AABB& aabb = submesh.BoundingBox;
			glm::mat4 aabbTransform = transform * submesh.Transform;
			DrawAABB(aabb, aabbTransform);
		}
	}

	void Renderer2D::DrawAABB(const AABB& aabb, const glm::mat4& transform, const glm::vec4& color)
	{
		glm::vec4 corners[8] =
		{
			transform * glm::vec4 { aabb.Min.x, aabb.Min.y, aabb.Max.z, 1.0f },
			transform * glm::vec4 { aabb.Min.x, aabb.Max.y, aabb.Max.z, 1.0f },
			transform * glm::vec4 { aabb.Max.x, aabb.Max.y, aabb.Max.z, 1.0f },
			transform * glm::vec4 { aabb.Max.x, aabb.Min.y, aabb.Max.z, 1.0f },

			transform * glm::vec4 { aabb.Min.x, aabb.Min.y, aabb.Min.z, 1.0f },
			transform * glm::vec4 { aabb.Min.x, aabb.Max.y, aabb.Min.z, 1.0f },
			transform * glm::vec4 { aabb.Max.x, aabb.Max.y, aabb.Min.z, 1.0f },
			transform * glm::vec4 { aabb.Max.x, aabb.Min.y, aabb.Min.z, 1.0f }
		};

		for (uint32_t i = 0; i < 4; i++)
			DrawLine(corners[i], corners[(i + 1) % 4], color);

		for (uint32_t i = 0; i < 4; i++)
			DrawLine(corners[i + 4], corners[((i + 1) % 4) + 4], color);

		for (uint32_t i = 0; i < 4; i++)
			DrawLine(corners[i], corners[i + 4], color);
	}

	void Renderer2D::DrawString(const std::string& string, const glm::vec3& position, float maxWidth, const glm::vec4& color)
	{
		DrawString(string, Font::GetDefaultFont(), position, maxWidth, color);
	}

	void Renderer2D::DrawString(const std::string& string, const Ref<Font>& font, const glm::vec3& position, float maxWidth, const glm::vec4& color)
	{
		DrawString(string, font, glm::translate(glm::mat4(1.0f), position), maxWidth, color);
	}

	void Renderer2D::DrawString(const std::string& string, const Ref<Font>& font, const glm::mat4& transform, float maxWidth, const glm::vec4& color, float lineHeightOffset, float kerningOffset)
	{
		if (string.empty())
			return;

		if (m_TextIndexCount >= MaxQuadIndices)
			FlushAndReset();

		std::u32string utf32String = Utils::To_UTF32(string);

		Ref<Texture2D> fontAtlas = font->GetFontAtlas();
		AR_CORE_ASSERT(fontAtlas);

		// textureIndex is the index that will be submitted in the VBO with everything and then passed on to the fragment shader so 
		// that the shader knows which index from the sampler to sample from.
		
		// So here we need to find the texture index of the passed index and check if it has already been used.
		// If it the case where it has been used before, the index will be already found in the array and we just return the index
		float textureIndex = 0.0f;
		for (uint32_t i = 0; i < m_FontTextureSlots.size(); i++)
		{
			if (m_FontTextureSlots[i] == fontAtlas)
			{
				textureIndex = (float)i;
				break;
			}
		}

		// m_FontTextureSlotIndex is the next available index in the sampler
		// If the case happens that it has never been used before, here we just add that index to the array so that it can be used later.
		if (textureIndex == 0.0f)
		{
			if (m_FontTextureSlotIndex >= MaxQuadTextureSlots)
				FlushAndReset();

			textureIndex = (float)m_FontTextureSlotIndex;
			m_FontTextureSlots[m_FontTextureSlotIndex] = fontAtlas;
			m_FontTextureSlotIndex++;
		}

		const msdf_atlas::FontGeometry& fontGeometry = font->GetMSDFData()->FontGeometry;
		const msdfgen::FontMetrics& metrics = fontGeometry.getMetrics();

		std::vector<int> nextLines;
		{
			double x = 0.0;
			double fsScale = 1.0 / (metrics.ascenderY - metrics.descenderY);
			double y = -fsScale * metrics.ascenderY;
			int lastSpace = -1;
			for (int i = 0; i < utf32String.size(); i++)
			{
				char32_t character = utf32String[i];
				if (character == '\n')
				{
					x = 0;
					y -= fsScale * metrics.lineHeight + lineHeightOffset;
					continue;
				}

				const msdf_atlas::GlyphGeometry* glyph = fontGeometry.getGlyph(character);
				if (!glyph)
					glyph = fontGeometry.getGlyph('?');
				if (!glyph)
					continue;

				if (character != ' ')
				{
					// Calc geo
					double pl;
					double pb;
					double pr;
					double pt;

					glyph->getQuadPlaneBounds(pl, pb, pr, pt);
					glm::vec2 quadMin = glm::vec2((float)pl, (float)pb);
					glm::vec2 quadMax = glm::vec2((float)pr, (float)pt);

					quadMin *= fsScale;
					quadMax *= fsScale;
					quadMin += glm::vec2(x, y);
					quadMax += glm::vec2(x, y);

					if (quadMax.x > maxWidth && lastSpace != -1)
					{
						i = lastSpace;
						nextLines.emplace_back(lastSpace);
						lastSpace = -1;
						x = 0;
						y -= fsScale * metrics.lineHeight + lineHeightOffset;
					}
				}
				else
				{
					lastSpace = i;
				}

				double advance = glyph->getAdvance();
				fontGeometry.getAdvance(advance, character, utf32String[i + 1]);
				x += fsScale * advance + kerningOffset;
			}
		}

		{
			double x = 0.0;
			double fsScale = 1.0 / (metrics.ascenderY - metrics.descenderY);
			double y = 0.0; // -fsScale * metrics.ascenderY;
			for (int i = 0; i < utf32String.size(); i++)
			{
				char32_t character = utf32String[i];
				if (character == '\n' || Utils::NextLine(i, nextLines))
				{
					x = 0;
					y -= fsScale * metrics.lineHeight + lineHeightOffset;
					continue;
				}

				const msdf_atlas::GlyphGeometry* glyph = fontGeometry.getGlyph(character);
				if (!glyph)
					glyph = fontGeometry.getGlyph('?');
				if (!glyph)
					continue;

				double l;
				double b;
				double r;
				double t;
				glyph->getQuadAtlasBounds(l, b, r, t);
				
				double pl;
				double pb;
				double pr;
				double pt;
				glyph->getQuadPlaneBounds(pl, pb, pr, pt);

				pl *= fsScale;
				pb *= fsScale;
				pr *= fsScale;
				pt *= fsScale;

				pl += x;
				pb += y;
				pr += x;
				pt += y;

				double texelWidth = 1.0 / fontAtlas->GetWidth();
				double texelHeight = 1.0 / fontAtlas->GetHeight();

				l *= texelWidth;
				b *= texelHeight;
				r *= texelWidth;
				t *= texelHeight;

				m_TextVertexBufferPtr->Position = transform * glm::vec4(pl, pb, 0.0f, 1.0f);;
				m_TextVertexBufferPtr->Color = color;
				m_TextVertexBufferPtr->TexCoord = { l, b };
				m_TextVertexBufferPtr->TextureIndex = textureIndex;
				m_TextVertexBufferPtr++;

				m_TextVertexBufferPtr->Position = transform * glm::vec4(pl, pt, 0.0f, 1.0f);;
				m_TextVertexBufferPtr->Color = color;
				m_TextVertexBufferPtr->TexCoord = { l, t };
				m_TextVertexBufferPtr->TextureIndex = textureIndex;
				m_TextVertexBufferPtr++;

				m_TextVertexBufferPtr->Position = transform * glm::vec4(pr, pt, 0.0f, 1.0f);;
				m_TextVertexBufferPtr->Color = color;
				m_TextVertexBufferPtr->TexCoord = { r, t };
				m_TextVertexBufferPtr->TextureIndex = textureIndex;
				m_TextVertexBufferPtr++;

				m_TextVertexBufferPtr->Position = transform * glm::vec4(pr, pb, 0.0f, 1.0f);;
				m_TextVertexBufferPtr->Color = color;
				m_TextVertexBufferPtr->TexCoord = { r, b };
				m_TextVertexBufferPtr->TextureIndex = textureIndex;
				m_TextVertexBufferPtr++;

				m_TextIndexCount += 6;

				double advance = glyph->getAdvance();
				fontGeometry.getAdvance(advance, character, utf32String[i + 1]);
				x += fsScale * advance + kerningOffset;

				m_Stats.QuadCount++;
			}
		}
	}

	void Renderer2D::SetLineWidth(float lineWidth)
	{
		m_LineWidth = lineWidth;
	}

	void Renderer2D::ResetStats()
	{
		memset(&m_Stats, 0, sizeof(Statistics));
	}

	const Renderer2D::Statistics& Renderer2D::GetStats()
	{
		return m_Stats;
	}

}