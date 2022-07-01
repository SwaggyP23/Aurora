#pragma once

#include "EditorCamera.h"
#include "VertexArray.h"
#include "Shader.h"

class Renderer
{
public:
	static void BeginScene(const std::shared_ptr<EditorCamera>& camera);
	static void EndScene();
	
	static void DrawQuad(const std::shared_ptr<Shader>& shader, const glm::mat4& model, const std::shared_ptr<VertexArray>& VAO);
	static void DrawSphere(const std::shared_ptr<Shader>& shader, const glm::mat4& model, const std::shared_ptr<VertexArray>& VAO);

private:
	static glm::mat4 m_Projection;
	static glm::mat4 m_View;
	static glm::vec3 m_CamPos;

};