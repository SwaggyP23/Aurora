#include "OGLpch.h"
#include "RenderCommand.h"

void RenderCommand::setClearColor(const glm::vec4& color)
{
	glClearColor(color.r, color.g, color.b, color.w);
}

void RenderCommand::Clear()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void RenderCommand::SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
{
	glViewport(x, y, width, height);
}

void RenderCommand::ActivateTextures(const std::vector<std::shared_ptr<Texture>>& texts)
{
	int i = 0;
	for (const auto& texture : texts)
	{
		glActiveTexture(GL_TEXTURE0 + i);
		texture->bind();
		i++;
	}
}

void RenderCommand::DrawIndexed(const std::shared_ptr<VertexArray>& vertexArray, bool state)
{
	if (!state)
		glDrawElements(GL_TRIANGLES, (GLsizei)vertexArray->getIndexBuffer()->getCount(), GL_UNSIGNED_INT, nullptr);
	else
		glDrawElements(GL_TRIANGLE_STRIP, (GLsizei)vertexArray->getIndexBuffer()->getCount(), GL_UNSIGNED_INT, nullptr);
}