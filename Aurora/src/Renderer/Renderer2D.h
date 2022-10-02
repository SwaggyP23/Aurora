#pragma once

#include "Core/Base.h"
#include "Editor/EditorCamera.h"
#include "Scene/SceneCamera.h"
#include "RenderCommand.h"
#include "Graphics/VertexArray.h"
#include "Graphics/Texture.h"
#include "Graphics/Shader.h"
#include "Graphics/Material.h"
#include "Graphics/RenderPass.h"
#include "Graphics/UniformBuffer.h"

/*
 * The way this batching works is that it batches all the elements in one VertexBuffer and submits it every frame. If the amount 
 * of elements exceeds the maximum amount specified by the capabilities of the gpu, we flush and start a new batch thus increasing
 * the draw calls if necessary. OR if the amount of used textures exceeds the maximum amount allowed (32 in my case) the renderer
 * also flushes and starts another batch. And to reuse the old textures they should be resubmitted!
 * And from the available 32 texture slots, slot 0 is reserved by the white texture in the case we want to draw just plain colors
 * we can submit texture index 0 and the sampler2D will sample from a white texture (1.0f) thus allowing for plain colors to appear.
 * 
 * TODO: REWORK FROM THE GROUND UP!!!
 */

namespace Aurora {

	struct Renderer2DSpecification
	{
		bool SwapChainTarget = false;
	};

	class Renderer2D : public RefCountedObject
	{
	public:
		Renderer2D(const Renderer2DSpecification& spec = Renderer2DSpecification());
		virtual ~Renderer2D();

		static Ref<Renderer2D> Create();

		void Init();
		void ShutDown();

		void SetTargetRenderPass(Ref<RenderPass> renderPass);

		void BeginScene(const glm::mat4& viewProj, const glm::mat4& view, bool depthTest = true);
		void EndScene();

		void DrawQuad(const glm::vec3& position, const glm::vec2& scale, const glm::vec4& color);
		void DrawQuad(const glm::vec3& position, const glm::vec2& scale, const Ref<Texture2D>& texture, float tiling = 1.0f, const glm::vec4& tintcolor = glm::vec4(1.0f));

		// Rotated Quad function are given the rotation in radians directly since that is what is stored in the TransformComponent
		void DrawRotatedQuad(const glm::vec3& position, const glm::vec3& rotations, const glm::vec2& scale, const glm::vec4& color);
		void DrawRotatedQuad(const glm::vec3& position, const glm::vec3& rotations, const glm::vec2& scale, const Ref<Texture2D>& texture, float tiling = 10.0f, const glm::vec4& tintColor = glm::vec4(1.0f));

		struct Statistics
		{
			uint32_t DrawCalls = 0;
			uint32_t QuadCount = 0;

			uint32_t GetTotalVertexCount() { return QuadCount * 4; }
			uint32_t GetTotalIndexCount() { return QuadCount * 6; }
			uint32_t GetTotalVertexBufferMemory() { return GetTotalVertexCount() * 11 * 4; }
		};

		void SetLineWidth(float lineWidth);

		void ResetStats();
		const Statistics& GetStats();

	private:
		void FlushAndReset();

	private:
		Renderer2DSpecification m_Specification;
		Ref<RenderPass> m_TargetRenderPass;

		struct QuadVertex
		{
			glm::vec3 Position;
			glm::vec4 Color;
			glm::vec2 TexCoord;
			float TextureIndex;
			float TilingFactor;
		};

		static const uint32_t MaxQuads = 10000;
		static const uint32_t MaxVertices = MaxQuads * 4;
		static const uint32_t MaxIndices = MaxQuads * 6;
		static const uint32_t MaxTextureSlots = 32; // TODO: Use the RendererCaps for this

		Ref<Texture2D> m_WhiteTexture = nullptr;

		Ref<VertexArray> m_QuadVertexArray;
		Ref<VertexBuffer> m_QuadVertexBuffer;
		Ref<IndexBuffer> m_QuadIndexBuffer;
		Ref<Material> m_QuadMaterial;

		uint32_t m_QuadIndexCount = 0;
		QuadVertex* m_QuadVertexBufferBase = nullptr;
		QuadVertex* m_QuadVertexBufferPtr = nullptr;

		std::array<Ref<Texture2D>, MaxTextureSlots> m_TextureSlots;
		uint32_t m_TextureSlotIndex = 1; // 0 is reserved for the white texture

		glm::vec4 m_QuadVertexPositions[4];
		glm::vec2 m_TextureCoords[4];
		static const uint32_t QuadVertexCount = 4;

		glm::mat4 m_CameraViewProj;
		glm::mat4 m_CameraView;
		bool m_DepthTest = true;

		float m_LineWidth = 1.0f;

		Statistics m_Stats;

		struct UBCamera
		{
			glm::mat4 ViewProjection;
		};

		Ref<UniformBuffer> m_CamUniformBuffer;

	};

}