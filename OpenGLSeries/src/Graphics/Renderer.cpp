#include "OGLpch.h"
#include "Renderer.h"
#include "RenderCommand.h"
#include "Application.h"

glm::mat4 Renderer::m_ViewProjection;

void Renderer::BeginScene(const std::shared_ptr<EditorCamera>& camera)
{
	m_ViewProjection = camera->GetProjection() * camera->GetViewMatrix();
}

void Renderer::EndScene()
{
	// ????
}

void Renderer::onWindowResize(uint32_t width, uint32_t height)
{
	RenderCommand::SetViewport(0, 0, width, height);
}

// For i will not take in the shader since i am already setting uniform outside the renderer
void Renderer::DrawQuad(const std::shared_ptr<Shader>& shader, const glm::mat4& model, const std::shared_ptr<VertexArray>& VAO)
{	
	shader->setUniformMat4("vw_pr_matrix", m_ViewProjection);
	shader->setUniformMat4("ml_matrix", model);
	shader->setUniformMat3("normalMatrix", glm::transpose(glm::inverse(model)));

	VAO->bind();
	RenderCommand::DrawIndexed(VAO);
}

void Renderer::DrawSphere(const std::shared_ptr<Shader>& shader, const glm::mat4& model, const std::shared_ptr<VertexArray>& VAO)
{
	shader->setUniformMat4("vw_pr_matrix", m_ViewProjection);
	shader->setUniformMat4("ml_matrix", model);
	shader->setUniformMat3("normalMatrix", glm::transpose(glm::inverse(model)));

	VAO->bind();
	RenderCommand::DrawIndexed(VAO, true);
}