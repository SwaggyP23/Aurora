#include "Aurorapch.h"
#include "RenderCommand.h"

#include <glad/glad.h>

namespace Aurora {

	namespace Utils {

		static GLenum GLFunctionFromEnum(OpenGLFunction func)
		{
			switch (func)
			{
			    case OpenGLFunction::None:                         return GL_NONE;
			    case OpenGLFunction::Never:                        return GL_NEVER;
			    case OpenGLFunction::Less:                         return GL_LESS;
			    case OpenGLFunction::Equal:                        return GL_EQUAL;
			    case OpenGLFunction::LessOrEqual:                  return GL_LEQUAL;
			    case OpenGLFunction::Greater:                      return GL_GREATER;
			    case OpenGLFunction::GreaterOrEqual:               return GL_GEQUAL;
			    case OpenGLFunction::NotEqual:                     return GL_NOTEQUAL;
			    case OpenGLFunction::Always:                       return GL_ALWAYS;
				case OpenGLFunction::Zero:                         return GL_ZERO;
				case OpenGLFunction::One:                          return GL_ONE;
				case OpenGLFunction::SrcColor:                     return GL_SRC_COLOR;
				case OpenGLFunction::OneMinusSrcColor:             return GL_ONE_MINUS_SRC_COLOR;
				case OpenGLFunction::DstColor:                     return GL_DST_COLOR;
				case OpenGLFunction::OneMinusDstColor:             return GL_ONE_MINUS_DST_COLOR;
				case OpenGLFunction::SrcAlpha:                     return GL_SRC_ALPHA;
				case OpenGLFunction::OneMinusSrcAlpha:             return GL_ONE_MINUS_SRC_ALPHA;
				case OpenGLFunction::Front:                        return GL_FRONT;
				case OpenGLFunction::Back:                         return GL_BACK;
				case OpenGLFunction::FrontAndBack:                 return GL_FRONT_AND_BACK;
			}

			AR_CORE_ASSERT(false, "Unkown Function!");
			return 0;
		}

		static GLenum GLFeatureFromFeatureControl(FeatureControl feat)
		{
			switch (feat)
			{
			    case FeatureControl::None:                return GL_NONE;
			    case FeatureControl::DepthTesting:        return GL_DEPTH_TEST;
			    case FeatureControl::Culling:             return GL_CULL_FACE;
			    case FeatureControl::Blending:            return GL_BLEND;
			    case FeatureControl::StencilTesting:      return GL_STENCIL_TEST;
			}

			AR_CORE_ASSERT(false, "Unkown Feature to enable!");
			return 0;
		}

	}

	void RenderCommand::Init()
	{
		// These are the defualt types of function and enables that the editor comes with right out of the box, things could be added or removed using the settings panel
		AR_PROFILE_FUNCTION();

		Enable(FeatureControl::DepthTesting);
		SetFeatureControlFunction(FeatureControl::DepthTesting, OpenGLFunction::Less);

		Enable(FeatureControl::Culling);
		SetFeatureControlFunction(FeatureControl::Culling, OpenGLFunction::Back);

		Enable(FeatureControl::Blending);
		SetFeatureControlFunction(FeatureControl::Blending, OpenGLFunction::OneMinusSrcAlpha);
	}

	void RenderCommand::ShutDown()
	{
	}

	void RenderCommand::Enable(FeatureControl feature)
	{
		glEnable(Utils::GLFeatureFromFeatureControl(feature));
	}

	void RenderCommand::Disable(FeatureControl feature)
	{
		glDisable(Utils::GLFeatureFromFeatureControl(feature));
	}

	void RenderCommand::SetFeatureControlFunction(FeatureControl feature, OpenGLFunction function)
	{
		switch (feature)
		{
		    case Aurora::FeatureControl::None:                  break;
		    case Aurora::FeatureControl::DepthTesting:          glDepthFunc(Utils::GLFunctionFromEnum(function)); break;
		    case Aurora::FeatureControl::Culling:               glCullFace(Utils::GLFunctionFromEnum(function)); break;
		    case Aurora::FeatureControl::Blending:              glBlendFunc(GL_SRC_ALPHA, Utils::GLFunctionFromEnum(function)); break;
		    case Aurora::FeatureControl::StencilTesting:        AR_CORE_ASSERT(false, "Not Implemented!"); break;
		}
	}

	void RenderCommand::SetClearColor(const glm::vec4& color)
	{
		glClearColor(color.r, color.g, color.b, color.w);
	}

	void RenderCommand::Clear()
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	void RenderCommand::SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
	{
		glViewport(x, y, width, height);
	}

	void RenderCommand::DrawIndexed(const Ref<VertexArray>& vertexArray, uint32_t indexCount)
	{
		AR_PROFILE_FUNCTION();

		vertexArray->bind();
		uint32_t count = indexCount ? vertexArray->GetIndexBuffer()->GetCount() : indexCount;
		glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, nullptr);
	}

}