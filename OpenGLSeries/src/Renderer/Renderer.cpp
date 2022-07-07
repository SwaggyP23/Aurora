#include "OGLpch.h"
#include "RendererPorperties.h"
#include "Renderer.h"
#include "Renderer3D.h"
#include "RenderCommand.h"
#include "Core/Application.h"

Scope<Renderer::SceneData> Renderer::s_SceneData = CreateScope<Renderer::SceneData>();

void Renderer::Init()
{
	PROFILE_FUNCTION();

	RendererProperties::Init();
	Renderer3D::Init();
	RenderCommand::Init();
}

void Renderer::ShutDown()
{
	// PROFILE_FUNCTION(); Currently not needed since Application destructor profiles it

	Renderer3D::ShutDown();
	RenderCommand::ShutDown();
	RendererProperties::ShutDown();
}

void Renderer::BeginScene(const Ref<EditorCamera>& camera)
{
	PROFILE_FUNCTION();

	s_SceneData->viewProjectionMatrix = camera->GetProjection() * camera->GetViewMatrix();;
}

void Renderer::BeginScene(const Ref<OrthoGraphicCamera>& camera)
{
	PROFILE_FUNCTION();

	s_SceneData->viewProjectionMatrix = camera->GetViewProjection();
}

void Renderer::EndScene()
{
}

void Renderer::onWindowResize(uint32_t width, uint32_t height)
{
	PROFILE_FUNCTION();

	RenderCommand::SetViewport(0, 0, width, height);
}

void Renderer::DrawQuad(const Ref<Shader>& shader, const glm::mat4& model, const Ref<VertexArray>& VAO)
{
	PROFILE_FUNCTION();

	shader->setUniformMat4("vw_pr_matrix", s_SceneData->viewProjectionMatrix);
	shader->setUniformMat4("ml_matrix", model);
	shader->setUniformMat3("normalMatrix", glm::transpose(glm::inverse(model)));

	VAO->bind();
	RenderCommand::DrawIndexed(VAO, true);
}

void Renderer::DrawSphere(const Ref<Shader>& shader, const glm::mat4& model, const Ref<VertexArray>& VAO)
{
	PROFILE_FUNCTION();

	shader->setUniformMat4("vw_pr_matrix", s_SceneData->viewProjectionMatrix);
	shader->setUniformMat4("ml_matrix", model);
	shader->setUniformMat3("normalMatrix", glm::transpose(glm::inverse(model)));

	VAO->bind();
	RenderCommand::DrawIndexed(VAO, false);
}