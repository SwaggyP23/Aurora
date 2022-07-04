#include "OGLpch.h"
#include "Renderer.h"
#include "RenderCommand.h"
#include "Core/Application.h"

Scope<Renderer::SceneData> Renderer::s_SceneData = CreateScope<Renderer::SceneData>();

void Renderer::Init()
{
	RenderCommand::Init();
}

void Renderer::ShutDown()
{
	RenderCommand::ShutDown();
}

void Renderer::BeginScene(const Ref<EditorCamera>& camera)
{
	s_SceneData->viewProjectionMatrix = camera->GetProjection() * camera->GetViewMatrix();;
}

void Renderer::BeginScene(const Ref<OrthoGraphicCamera>& camera)
{
	s_SceneData->viewProjectionMatrix = camera->GetViewProjection();
}

void Renderer::EndScene()
{
}

void Renderer::onWindowResize(uint32_t width, uint32_t height)
{
	RenderCommand::SetViewport(0, 0, width, height);
}

// For i will not take in the shader since i am already setting uniform outside the renderer
void Renderer::DrawQuad(const Ref<Shader>& shader, const glm::mat4& model, const Ref<VertexArray>& VAO)
{	
	shader->setUniformMat4("vw_pr_matrix", s_SceneData->viewProjectionMatrix);
	shader->setUniformMat4("ml_matrix", model);
	shader->setUniformMat3("normalMatrix", glm::transpose(glm::inverse(model)));

	VAO->bind();
	RenderCommand::DrawIndexed(VAO);
}

void Renderer::DrawSphere(const Ref<Shader>& shader, const glm::mat4& model, const Ref<VertexArray>& VAO)
{
	shader->setUniformMat4("vw_pr_matrix", s_SceneData->viewProjectionMatrix);
	shader->setUniformMat4("ml_matrix", model);
	shader->setUniformMat3("normalMatrix", glm::transpose(glm::inverse(model)));

	VAO->bind();
	RenderCommand::DrawIndexed(VAO, true);
}