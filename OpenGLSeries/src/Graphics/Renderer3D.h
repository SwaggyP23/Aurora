#pragma once

#include "EditorCamera.h"
#include "OrthographicCamera.h"

#include "RenderCommand.h"

#include "VertexArray.h"
#include "Texture.h"
#include "Shader.h"

// Should be one uniform buffer for quads, another for spheres and u get the idea.
// Should also be one and only one shader.

class Renderer3D
{
public:
	static void Init();
	static void ShutDown();

	static void BeginScene(const Ref<EditorCamera>& camera);
	static void BeginScene(const Ref<OrthoGraphicCamera>& camera);
	static void EndScene();

	static void DrawQuad(const glm::vec3& position, const glm::vec3 rotations, const glm::vec3& scale , const glm::vec4& color);
	static void DrawQuad(const glm::vec3& position, const glm::vec3 rotations, const glm::vec3& scale, const Ref<Texture>& texture);

};