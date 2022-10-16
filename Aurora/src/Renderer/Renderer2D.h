#pragma once

#include "Core/Base.h"
#include "Editor/EditorCamera.h"
#include "Scene/SceneCamera.h"
#include "RenderCommand.h"
#include "Graphics/Font.h"
#include "Graphics/Texture.h"
#include "Graphics/Shader.h"
#include "Graphics/Material.h"
#include "Graphics/Mesh.h"
#include "Graphics/RenderPass.h"
#include "Graphics/UniformBuffer.h"
#include "Graphics/VertexArray.h"

/*
 * The way this batching works is that it batches all the elements in one VertexBuffer and submits it every frame. If the amount
 * of elements exceeds the maximum amount specified by the capabilities of the gpu, we flush and start a new batch thus increasing
 * the draw calls if necessary. OR if the amount of used textures exceeds the maximum amount allowed (32 in my case) the renderer
 * also flushes and starts another batch. And to reuse the old textures they should be resubmitted!
 * And from the available 32 texture slots, slot 0 is reserved by the white texture in the case we want to draw just plain colors
 * we can submit texture index 0 and the sampler2D will sample from a white texture (1.0f) thus allowing for plain colors to appear.
 */

namespace Aurora {

	struct Renderer2DSpecification
	{
	};

	class Renderer2D : public RefCountedObject
	{
	public:
		Renderer2D(const Renderer2DSpecification& spec = Renderer2DSpecification());
		virtual ~Renderer2D();

		static Ref<Renderer2D> Create(const Renderer2DSpecification& spec = Renderer2DSpecification());

		void Init();
		void ShutDown();

		void SetTargetRenderPass(Ref<RenderPass> renderPass);

		Ref<Texture2D> GetDebugFinalImage();
		Ref<RenderPass> GetDebugRenderPass();

		void BeginScene(const glm::mat4& viewProj, const glm::mat4& view, bool depthTest = true);
		void EndScene();

		void DrawQuad(const glm::vec3& position, const glm::vec2& scale, const glm::vec4& color);
		void DrawQuad(const glm::vec3& position, const glm::vec2& scale, const Ref<Texture2D>& texture, float tiling = 1.0f, const glm::vec4& tintcolor = glm::vec4(1.0f));

		// Rotated Quad function are given the rotation in radians directly since that is what is stored in the TransformComponent
		void DrawRotatedQuad(const glm::vec3& position, const glm::vec3& rotations, const glm::vec2& scale, const glm::vec4& color);
		void DrawRotatedQuad(const glm::vec3& position, const glm::vec3& rotations, const glm::vec2& scale, const Ref<Texture2D>& texture, float tiling = 10.0f, const glm::vec4& tintColor = glm::vec4(1.0f));

		// Billboard Quads that always face the camera's view
		void DrawQuadBillboard(const glm::vec3& position, const glm::vec2& scale, const glm::vec4& color);
		void DrawQuadBillboard(const glm::vec3& position, const glm::vec2& scale, const Ref<Texture2D>& texture, float tiling = 1.0f, const glm::vec4& tintColor = glm::vec4(1.0f));

		// Thickness is between 0 and 1
		void DrawCircle(const glm::vec3& position, const glm::vec3& rotation, float radius, const glm::vec4& color);
		void DrawCircle(const glm::mat4& transform, const glm::vec4& color);
		void FillCircle(const glm::mat4& transform, const glm::vec4& color, float thickness = 0.05f); // Allows for non uniform scaling
		void FillCircle(const glm::vec2& position, float radius, const glm::vec4& color, float thickness = 0.05f);
		void FillCircle(const glm::vec3& position, float radius, const glm::vec4& color, float thickness = 0.05f);

		void DrawLine(const glm::vec3& p0, const glm::vec3& p1, const glm::vec4& color = glm::vec4(1.0f));

		void DrawRotatedRect(const glm::vec3& position, const glm::vec3& rotations, const glm::vec2& scale, const glm::vec4& color = glm::vec4(1.0f));

		void DrawAABB(Ref<StaticMesh> mesh, const glm::mat4& transform, const glm::vec4& color = glm::vec4(1.0f));
		void DrawAABB(const AABB& aabb, const glm::mat4& transform, const glm::vec4& color = glm::vec4(1.0f));

		void DrawString(const std::string& string, const glm::vec3& position, float maxWidth, const glm::vec4& color = glm::vec4(1.0f));
		void DrawString(const std::string& string, const Ref<Font>& font, const glm::vec3& position, float maxWidth, const glm::vec4& color = glm::vec4(1.0f));
		void DrawString(const std::string& string, const Ref<Font>& font, const glm::mat4& transform, float maxWidth, const glm::vec4& color = glm::vec4(1.0f), float lineHeightOffset = 0.0f, float kerningOffset = 0.0f);

		struct Statistics
		{
			uint32_t DrawCalls = 0;
			uint32_t QuadCount = 0;
			uint32_t LineCount = 0;

			uint32_t GetTotalVertexCount() { return QuadCount * 4; }
			uint32_t GetTotalIndexCount() { return QuadCount * 6; }
			uint32_t GetTotalVertexBufferMemory() { return GetTotalVertexCount() * 11 * 4; }
		};

		void SetLineWidth(float lineWidth);
		float GetLineWidth() const { return m_LineWidth; }

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

		struct LineVertex
		{
			glm::vec3 Position;
			glm::vec4 Color;
		};

		struct CircleVertex
		{
			glm::vec3 WorldPosition;
			float Thickness; // To pad the WorldPosition into a vec4
			glm::vec2 LocalPosition;
			glm::vec4 Color;
		};

		struct TextVertex
		{
			glm::vec3 Position;
			glm::vec4 Color;
			glm::vec2 TexCoord;
			float TextureIndex;
		};

		static const uint32_t MaxQuads = 10000;
		static const uint32_t MaxQuadVertices = MaxQuads * 4;
		static const uint32_t MaxQuadIndices = MaxQuads * 6;
		static const uint32_t MaxQuadTextureSlots = 32; // TODO: Use the RendererCaps for this

		static const uint32_t MaxLines = 2000;
		static const uint32_t MaxLineVertices = MaxLines * 2;
		static const uint32_t MaxLineIndices = MaxLines * 6;

		Ref<Texture2D> m_WhiteTexture = nullptr;

		// Quads...
		Ref<VertexArray> m_QuadVertexArray;
		Ref<VertexBuffer> m_QuadVertexBuffer;
		Ref<Material> m_QuadMaterial;

		uint32_t m_QuadIndexCount = 0;
		QuadVertex* m_QuadVertexBufferBase = nullptr;
		QuadVertex* m_QuadVertexBufferPtr = nullptr;

		// Lines...
		Ref<VertexArray> m_LineVertexArray;
		Ref<VertexBuffer> m_LineVertexBuffer;
		Ref<Material> m_LineMaterial;

		uint32_t m_LineIndexCount = 0;
		LineVertex* m_LineVertexBufferBase = nullptr;
		LineVertex* m_LineVertexBufferPtr = nullptr;

		// Circles...
		Ref<VertexArray> m_CircleVertexArray;
		Ref<VertexBuffer> m_CircleVertexBuffer;
		Ref<Material> m_CircleMaterial;

		uint32_t m_CircleIndexCount = 0;
		CircleVertex* m_CircleVertexBufferBase = nullptr;
		CircleVertex* m_CircleVertexBufferPtr = nullptr;

		// Text...
		Ref<VertexArray> m_TextVertexArray;
		Ref<VertexBuffer> m_TextVertexBuffer;
		Ref<Material> m_TextMaterial;
		std::array<Ref<Texture2D>, MaxQuadTextureSlots> m_FontTextureSlots;
		uint32_t m_FontTextureSlotIndex = 0;

		uint32_t m_TextIndexCount = 0;
		TextVertex* m_TextVertexBufferBase = nullptr;
		TextVertex* m_TextVertexBufferPtr = nullptr;

		std::array<Ref<Texture2D>, MaxQuadTextureSlots> m_TextureSlots;
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

		// For debugging only for now...
		Ref<RenderPass> m_DebugRenderPass;

	};

}