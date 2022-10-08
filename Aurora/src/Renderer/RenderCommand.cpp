#include "Aurorapch.h"
#include "RenderCommand.h"

#include <glad/glad.h>

namespace Aurora {

	namespace Utils {

		static GLenum GLFunctionFromEnum(Comparator func)
		{
			switch (func)
			{
			    case Comparator::None:                         return GL_NONE;
			    case Comparator::Never:                        return GL_NEVER;
			    case Comparator::Less:                         return GL_LESS;
			    case Comparator::Equal:                        return GL_EQUAL;
			    case Comparator::LessOrEqual:                  return GL_LEQUAL;
			    case Comparator::Greater:                      return GL_GREATER;
			    case Comparator::GreaterOrEqual:               return GL_GEQUAL;
			    case Comparator::NotEqual:                     return GL_NOTEQUAL;
			    case Comparator::Always:                       return GL_ALWAYS;
				case Comparator::Zero:                         return GL_ZERO;
				case Comparator::One:                          return GL_ONE;
				case Comparator::SrcColor:                     return GL_SRC_COLOR;
				case Comparator::OneMinusSrcColor:             return GL_ONE_MINUS_SRC_COLOR;
				case Comparator::DstColor:                     return GL_DST_COLOR;
				case Comparator::OneMinusDstColor:             return GL_ONE_MINUS_DST_COLOR;
				case Comparator::SrcAlpha:                     return GL_SRC_ALPHA;
				case Comparator::OneMinusSrcAlpha:             return GL_ONE_MINUS_SRC_ALPHA;
				case Comparator::Front:                        return GL_FRONT;
				case Comparator::Back:                         return GL_BACK;
				case Comparator::FrontAndBack:                 return GL_FRONT_AND_BACK;
			}

			AR_CORE_ASSERT(false, "Unknown Function!");
			return 0;
		}

		static GLenum GLEquationfromOpenGLEquation(BlendEquation eq)
		{
			switch (eq)
			{
			    case BlendEquation::Add:                 return GL_FUNC_ADD;
			    case BlendEquation::Subtract:            return GL_FUNC_SUBTRACT;
			    case BlendEquation::ReverseSubtract:     return GL_FUNC_REVERSE_SUBTRACT;
			    case BlendEquation::Minimum:             return GL_MIN;
			    case BlendEquation::Maximum:             return GL_MAX;
			}

			AR_CORE_ASSERT(false, "Unknown Function Equation!");
			return 0;
		}

		static GLenum GLFeatureFromFeatureControl(Capability feat)
		{
			switch (feat)
			{
			    case Capability::None:                return GL_NONE;
			    case Capability::DepthTesting:        return GL_DEPTH_TEST;
			    case Capability::Culling:             return GL_CULL_FACE;
			    case Capability::Blending:            return GL_BLEND;
			}

			AR_CORE_ASSERT(false, "Unknown Feature to enable!");
			return 0;
		}

	}

	void RenderCommand::Enable(Capability feature)
	{
		glEnable(Utils::GLFeatureFromFeatureControl(feature));
	}

	void RenderCommand::Disable(Capability feature)
	{
		glDisable(Utils::GLFeatureFromFeatureControl(feature));
	}

	void RenderCommand::SetBlendFunctionEquation(BlendEquation equation)
	{
		glBlendEquation(Utils::GLEquationfromOpenGLEquation(equation));
	}

	void RenderCommand::SetCapabilityFunction(Capability feature, Comparator function)
	{
		switch (feature)
		{
		    case Capability::None:                  break;
		    case Capability::DepthTesting:          glDepthFunc(Utils::GLFunctionFromEnum(function)); break;
		    case Capability::Culling:               glCullFace(Utils::GLFunctionFromEnum(function)); break;
		    case Capability::Blending:              glBlendFunc(GL_SRC_ALPHA, Utils::GLFunctionFromEnum(function)); break;
		}
	}

	void RenderCommand::SetClearColor(const glm::vec4& color)
	{
		glClearColor(color.r, color.g, color.b, color.w);
	}

	void RenderCommand::Clear()
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	}

}