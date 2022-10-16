#pragma once

#include "Renderer2D.h"

#include <vector>

namespace Aurora {

	// This is used for debug graphics mostly...
	// This will be exposed to the C# API...
	// TODO: Expand on this...
	class DebugRenderer : public RefCountedObject
	{
	public:
		using RenderQueueFunction = std::function<void(Ref<Renderer2D>)>;
		using RenderQueue = std::vector<RenderQueueFunction>;

	public:
		DebugRenderer() = default;
		~DebugRenderer() = default;

		static Ref<DebugRenderer> Create();

		void DrawLine(const glm::vec3& p0, const glm::vec3& p1, const glm::vec4& color = glm::vec4(1.0f));
		void DrawQuadBillboard(const glm::vec3& position, const glm::vec2& scale, const glm::vec4& color = glm::vec4(1.0f));
		// TODO: Add drawing with textures and everything...

		void SetLineWidth(float width);

		RenderQueue& GetRenderQueue() { return m_RenderQueue; }
		void ClearRenderQueue() { m_RenderQueue.clear(); }

	private:
		RenderQueue m_RenderQueue;

	};

}