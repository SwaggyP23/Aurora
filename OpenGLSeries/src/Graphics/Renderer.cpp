#include "OGLpch.h"
#include "Renderer.h"
#include "RenderCommand.h"
#include "Application.h"

glm::mat4 Renderer::m_Projection;
glm::mat4 Renderer::m_View;
glm::vec3 Renderer::m_CamPos;

void Renderer::BeginScene(const std::shared_ptr<EditorCamera>& camera)
{
	m_Projection = camera->GetProjection();
	m_View = camera->GetViewMatrix();
	m_CamPos = camera->GetPosition();
}

void Renderer::EndScene()
{
	// ????
}

// For i will not take in the shader since i am already setting uniform outside the renderer
void Renderer::DrawQuad(const std::shared_ptr<Shader>& shader, const glm::mat4& model, const std::shared_ptr<VertexArray>& VAO)
{
	ImGuiLayer* imgui = Application::getApp().getImGuiLayer();

	shader->bind();
	shader->setUniform4f("lightColor", imgui->getLightColor());
	shader->setUniform1f("blend", imgui->getBlend());
	shader->setUniform1f("ambientStrength", imgui->getAmbLight());
	shader->setUniform3f("src_pos", imgui->getLightTranslations());
	shader->setUniform3f("view_pos", m_CamPos);
	shader->setUniform4f("src_color", imgui->getLightColor());
	shader->setUniform4f("un_color", imgui->getUniColor());
	shader->setUniformMat4("pr_matrix", m_Projection);
	shader->setUniformMat4("vw_matrix", m_View);
	shader->setUniformMat4("ml_matrix", model);
	shader->setUniformMat3("normalMatrix", glm::transpose(glm::inverse(model)));

	VAO->bind();
	RenderCommand::DrawIndexed(VAO);
}

void Renderer::DrawSphere(const std::shared_ptr<Shader>& shader, const glm::mat4& model, const std::shared_ptr<VertexArray>& VAO)
{
	shader->bind();
	shader->setUniformMat4("pr_matrix", m_Projection);
	shader->setUniformMat4("vw_matrix", m_View);
	shader->setUniformMat4("ml_matrix", model);

	VAO->bind();
	RenderCommand::DrawIndexed(VAO, true);
}