#include "Aurorapch.h"
#include "RendererPorperties.h"
#include "Renderer.h"
#include "Renderer3D.h"
#include "RenderCommand.h"
#include "Core/Application.h"

namespace Aurora {

	Scope<Renderer::SceneData> Renderer::s_SceneData = CreateScope<Renderer::SceneData>();

	void Renderer::Init()
	{
		// Currently not in use
		// AR_PROFILE_FUNCTION();
		
		// Renderer3D::Init();
		// RenderCommand::Init();
	}

	void Renderer::ShutDown()
	{
		// Currently not in use
		// PROFILE_FUNCTION();

		// Renderer3D::ShutDown();
		// RenderCommand::ShutDown();
	}

	void Renderer::BeginScene(const Ref<EditorCamera>& camera)
	{
		AR_PROFILE_FUNCTION();

		s_SceneData->viewProjectionMatrix = camera->GetProjection() * camera->GetViewMatrix();;
	}

	void Renderer::EndScene()
	{
	}

	void Renderer::OnWindowResize(uint32_t width, uint32_t height)
	{
		AR_PROFILE_FUNCTION();

		RenderCommand::SetViewport(0, 0, width, height);
	}

	void Renderer::DrawQuad(const Ref<Shader>& shader, const glm::mat4& model, const Ref<VertexArray>& VAO)
	{
		AR_PROFILE_FUNCTION();

		shader->SetUniformMat4("vw_pr_matrix", s_SceneData->viewProjectionMatrix);
		shader->SetUniformMat4("ml_matrix", model);
		shader->SetUniformMat3("normalMatrix", glm::transpose(glm::inverse(model)));

		VAO->bind();
		RenderCommand::DrawIndexed(VAO, true);
	}

	void Renderer::DrawSphere(const Ref<Shader>& shader, const glm::mat4& model, const Ref<VertexArray>& VAO)
	{
		AR_PROFILE_FUNCTION();

		shader->SetUniformMat4("vw_pr_matrix", s_SceneData->viewProjectionMatrix);
		shader->SetUniformMat4("ml_matrix", model);
		shader->SetUniformMat3("normalMatrix", glm::transpose(glm::inverse(model)));

		VAO->bind();
		RenderCommand::DrawIndexed(VAO, false);
	}

}