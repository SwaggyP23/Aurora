#pragma once

#include <glm/glm.hpp>
#include <glad/glad.h>

#include "VertexArray.h"
#include "Texture.h"

class RenderCommand
{
public:
	static void setClearColor(const glm::vec4& color);
	static void Clear();

	static void ActivateTextures(const std::vector<std::shared_ptr<Texture>>& texts);
	static void DrawIndexed(const std::shared_ptr<VertexArray>& vertexArray, bool state = false);

};