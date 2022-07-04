#include "OGLpch.h"
#include "RenderCommand.h"

void RenderCommand::Init()
{
	glEnable(GL_DEPTH_TEST);
	// glEnable(GL_BLEND);		// To be used when i start with the blending chapter
	// glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void RenderCommand::ShutDown()
{
}

void RenderCommand::setClearColor(const glm::vec4& color)
{
	glClearColor(color.r, color.g, color.b, color.w);
}

void RenderCommand::Clear()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void RenderCommand::ActivateTextures(const std::vector<Ref<Texture>>& vec)
{
	int i = 0;
	for (auto tex : vec) {
		glActiveTexture(GL_TEXTURE0 + i);
		tex->bind();
		i++;
	}
}

void RenderCommand::SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
{
	glViewport(x, y, width, height);
}

void RenderCommand::DrawIndexed(const Ref<VertexArray>& vertexArray, bool state)
{
	if (!state)
		glDrawElements(GL_TRIANGLES, (GLsizei)vertexArray->getIndexBuffer()->getCount(), GL_UNSIGNED_INT, nullptr);
	else
		glDrawElements(GL_TRIANGLE_STRIP, (GLsizei)vertexArray->getIndexBuffer()->getCount(), GL_UNSIGNED_INT, nullptr);
}