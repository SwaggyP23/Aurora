#pragma once

#include "Graphics/VertexArray.h"
#include "Graphics/Texture.h"

#include <glm/glm.hpp>

namespace Aurora {

	enum class Capability : uint8_t
	{
		None = 0,
		Blending,
		Culling,
		DepthTesting
	};

	enum class BlendEquation : uint8_t
	{
		Add,
		Subtract,
		ReverseSubtract,
		Minimum,
		Maximum
	};

	enum class Comparator : uint8_t
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
		static void Enable(Capability feature); // TODO: Think of better naming for this api since its a disaster
		static void Disable(Capability feature);
		static void SetCapabilityFunction(Capability feature, Comparator function);
		static void SetBlendFunctionEquation(BlendEquation equation);

		static void SetClearColor(const glm::vec4& color);
		static void Clear();

	};

}