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
				case Comparator::Front:                        return GL_FRONT;
				case Comparator::Back:                         return GL_BACK;
				case Comparator::FrontAndBack:                 return GL_FRONT_AND_BACK;
			}

			AR_CORE_ASSERT(false, "Unknown Function!");
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

		static void SetBlendFunction(Comparator function)
		{
			switch (function)
			{
				case Comparator::OneZero:						glBlendFunc(GL_ONE, GL_ZERO); return;
				case Comparator::SrcAlphaOnceMinusSrcAlpha:		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); return;
				case Comparator::ZeroSrcColor:					glBlendFunc(GL_ZERO, GL_SRC_COLOR); return;
			}

			AR_CORE_ASSERT(false, "Unknown type!");
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

	void RenderCommand::SetCapabilityFunction(Capability feature, Comparator function)
	{
		switch (feature)
		{
		    case Capability::None:                  break;
		    case Capability::DepthTesting:          glDepthFunc(Utils::GLFunctionFromEnum(function)); break;
		    case Capability::Culling:               glCullFace(Utils::GLFunctionFromEnum(function)); break;
			case Capability::Blending:              Utils::SetBlendFunction(function); break;
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