#include "Aurorapch.h"
#include "DebugRenderer.h"

namespace Aurora {

	Ref<DebugRenderer> DebugRenderer::Create()
	{
		return CreateRef<DebugRenderer>();
	}

	void DebugRenderer::DrawLine(const glm::vec3& p0, const glm::vec3& p1, const glm::vec4& color)
	{
		m_RenderQueue.emplace_back([p0, p1, color](Ref<Renderer2D> renderer2D)
		{
			renderer2D->DrawLine(p0, p1, color);
		});
	}

	void DebugRenderer::DrawQuadBillboard(const glm::vec3& position, const glm::vec2& scale, const glm::vec4& color)
	{
		m_RenderQueue.emplace_back([position, scale, color](Ref<Renderer2D> renderer2D)
		{
			renderer2D->DrawQuadBillboard(position, scale, color);
		});
	}

	void DebugRenderer::SetLineWidth(float width)
	{
		m_RenderQueue.emplace_back([width](Ref<Renderer2D> renderer2D)
		{
			renderer2D->SetLineWidth(width);
		});
	}

}