#pragma once

#include <glm/glm.hpp>
#include <glad/glad.h>

#include "Graphics/VertexArray.h"
#include "Graphics/Texture.h"

class RenderCommand
{
public:
	static void Init();
	static void ShutDown();

	static void setClearColor(const glm::vec4& color);
	static void Clear();

	static void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height);

	static void DrawIndexed(const Ref<VertexArray>& vertexArray, bool state = false);

};