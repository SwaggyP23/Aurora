#pragma once

#include "Graphics/OrthoGraphicCamera.h"
#include "Graphics/EditorCamera.h"
#include "Graphics/VertexArray.h"
#include "Graphics/Shader.h"

class Renderer
{
public:
	static void Init();
	static void ShutDown();

	static void BeginScene(const Ref<EditorCamera>& camera);
	static void BeginScene(const Ref<OrthoGraphicCamera>& camera);
	static void EndScene();
	
	static void onWindowResize(uint32_t width, uint32_t height);

	static void DrawQuad(const Ref<Shader>& shader, const glm::mat4& model, const Ref<VertexArray>& VAO);
	static void DrawSphere(const Ref<Shader>& shader, const glm::mat4& model, const Ref<VertexArray>& VAO);

private:
	struct SceneData
	{
		glm::mat4 viewProjectionMatrix;
	};

	static Scope<SceneData> s_SceneData;

};