#pragma once

#include "Graphics/EditorCamera.h"
#include "Graphics/OrthographicCamera.h"

#include "RenderCommand.h"
#include "RendererPorperties.h"

#include "Graphics/VertexArray.h"
#include "Graphics/Texture.h"
#include "Graphics/Shader.h"

/*
 * The way this batching works is that it batches all the elements in one VertexBuffer and submits it every frame. If the amount 
 * of elements exceeds the maximum amount specified by the capabilities of the gpu, we flush and start a new batch thus increasing
 * the draw calls if necessary. OR if the amount of used textures exceeds the maximum amount allowed (32 in my case) the renderer
 * also flushes and starts another batch. And to reuse the old textures they should be resubmitted!
 * And from the available 32 texture slots, slot 0 is reserved by the white texture in the case we want to draw just plain colors
 * we can submit texture index 0 and the sampler2D will sample from a white texture (1.0f) thus allowing for plain colors to appear.
 */

namespace Aurora {

	class Renderer3D
	{
	public:
		static void Init();
		static void ShutDown();

		static void BeginScene(const Ref<EditorCamera>& camera);
		static void BeginScene(const Ref<OrthoGraphicCamera>& camera);
		static void EndScene();
		static void Flush();

		static void DrawQuad(const glm::vec3& position, const glm::vec3& scale, const glm::vec4& color, int light = 0);
		static void DrawQuad(const glm::vec3& position, const glm::vec3& scale, const Ref<Texture>& texture, float tiling = 1.0f, const glm::vec4& tintcolor = glm::vec4(1.0f));

		static void DrawRotatedQuad(const glm::vec3& position, const glm::vec3 rotations, const glm::vec3& scale, const glm::vec4& color, int light = 0);
		static void DrawRotatedQuad(const glm::vec3& position, const glm::vec3 rotations, const glm::vec3& scale, const Ref<Texture>& texture, float tiling = 10.0f, const glm::vec4& tintColor = glm::vec4(1.0f));

		struct Statistics
		{
			uint32_t DrawCalls = 0;
			uint32_t QuadCount = 0;

			uint32_t GetTotalVertexCount() { return QuadCount * 24; }
			uint32_t GetTotalIndexCount() { return QuadCount * 36; }
			uint32_t GetTotalVertexBufferMemory() { return GetTotalVertexCount() * 11 * 4; }
		};

		static void ResetStats();
		static Statistics& GetStats();

	private:
		static void StartBatch();
		static void NextBatch();

	};

}