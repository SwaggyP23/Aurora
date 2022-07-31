#pragma once

#include "Graphics/VertexArray.h"
#include "Graphics/Texture.h"

#include <glm/glm.hpp>

namespace Aurora {

	enum class FeatureControl
	{
		None = 0,
		Blending,
		Culling,
		DepthTesting,
		StencilTesting // Not implemented yet!
	};

	enum class OpenGLEquation
	{
		Add,
		Subtract,
		ReverseSubtract,
		Minimum,
		Maximum
	};

	enum class OpenGLFunction
	{
		None = 0,

		// Depth Testing
		Never,
		Less,
		Equal,
		LessOrEqual,
		Greater,
		GreaterOrEqual,
		NotEqual,
		Always,

		// Blending
		Zero,
		One,
		SrcColor,
		OneMinusSrcColor,
		DstColor,
		OneMinusDstColor,
		SrcAlpha,
		OneMinusSrcAlpha,

		// Culling
		Front,
		Back,
		FrontAndBack

		// Stencil has the same function types as depth testing so it is not necessary to specify more enums for it
	};

	class RenderCommand
	{
	public:
		static void Init();
		static void ShutDown();

		static void Enable(FeatureControl feature); // TODO: Think of better naming for this api since its a disaster
		static void Disable(FeatureControl feature);
		static void SetFeatureControlFunction(FeatureControl feature, OpenGLFunction function);
		static void SetBlendFunctionEquation(OpenGLEquation equation);

		static void SetClearColor(const glm::vec4& color);
		static void Clear();

		static void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height);

		static void DrawIndexed(const Ref<VertexArray>& vertexArray, uint32_t indexCount = 0);

	};

}