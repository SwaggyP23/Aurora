#pragma once

#include "Graphics/EditorCamera.h"
#include "Graphics/OrthographicCamera.h"

#include "RenderCommand.h"
#include "RendererPorperties.h"

#include "Graphics/VertexArray.h"
#include "Graphics/Texture.h"
#include "Graphics/Shader.h"

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
	static void Flush();

	static void DrawQuad(const glm::vec3& position, const glm::vec3& size, const glm::vec4& color);
	static void DrawQuad(const glm::vec3& position, const glm::vec3& size, const Ref<Texture>& texture, float tiling = 1.0f, const glm::vec4& tintcolor = glm::vec4(1.0f));

	static void DrawRotatedQuad(const glm::vec3& position, const glm::vec3 rotations, const glm::vec3& scale , const glm::vec4& color);
	static void DrawRotatedQuad(const glm::vec3& position, const glm::vec3 rotations, const glm::vec3& scale, const Ref<Texture>& texture, float tiling = 10.0f);

};