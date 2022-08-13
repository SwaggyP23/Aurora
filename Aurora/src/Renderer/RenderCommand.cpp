#include "Aurorapch.h"
#include "RenderCommand.h"

#include <glad/glad.h>

namespace Aurora {

	RenderFlags RenderCommand::m_Flags;

	namespace Utils {

		static GLenum GLTypeFromRenderFlags(RenderFlags flag)
		{
			switch (flag)
			{
			    case Aurora::RenderFlags::None:            return GL_NONE;
			    case Aurora::RenderFlags::Fill:       return GL_FILL;
			    case Aurora::RenderFlags::WireFrame:       return GL_LINE;
			    case Aurora::RenderFlags::Vertices:        return GL_POINT;
			}

			AR_CORE_ASSERT(false, "Unkown Render Flag!");
			return 0;
		}

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

		static GLenum GLEquationfromOpenGLEquation(OpenGLEquation eq)
		{
			switch (eq)
			{
			    case Aurora::OpenGLEquation::Add:                 return GL_FUNC_ADD;
			    case Aurora::OpenGLEquation::Subtract:            return GL_FUNC_SUBTRACT;
			    case Aurora::OpenGLEquation::ReverseSubtract:     return GL_FUNC_REVERSE_SUBTRACT;
			    case Aurora::OpenGLEquation::Minimum:             return GL_MIN;
			    case Aurora::OpenGLEquation::Maximum:             return GL_MAX;
			}

			AR_CORE_ASSERT(false, "Unkown Function Equation!");
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

		//Enable(FeatureControl::StencilTesting); // Not implemented currently
		//SetFeatureControlFunction(FeatureControl::StencilTesting, OpenGLFunction::Always);

		Enable(FeatureControl::Culling);
		SetFeatureControlFunction(FeatureControl::Culling, OpenGLFunction::Back);

		Enable(FeatureControl::Blending);
		SetFeatureControlFunction(FeatureControl::Blending, OpenGLFunction::OneMinusSrcAlpha);

		m_Flags = RenderFlags::Fill;
	}

	void RenderCommand::ShutDown()
	{
	}

	void RenderCommand::SetRenderFlag(RenderFlags flag)
	{
		m_Flags = flag;
		glPolygonMode(GL_FRONT_AND_BACK, Utils::GLTypeFromRenderFlags(flag));
	}

	void RenderCommand::Enable(FeatureControl feature)
	{
		glEnable(Utils::GLFeatureFromFeatureControl(feature));
	}

	void RenderCommand::Disable(FeatureControl feature)
	{
		glDisable(Utils::GLFeatureFromFeatureControl(feature));
	}

	void RenderCommand::SetBlendFunctionEquation(OpenGLEquation equation)
	{
		glBlendEquation(Utils::GLEquationfromOpenGLEquation(equation));
	}

	void RenderCommand::SetFeatureControlFunction(FeatureControl feature, OpenGLFunction function)
	{
		switch (feature)
		{
		    case Aurora::FeatureControl::None:                  break;
		    case Aurora::FeatureControl::DepthTesting:          glDepthFunc(Utils::GLFunctionFromEnum(function)); break;
		    case Aurora::FeatureControl::Culling:               glCullFace(Utils::GLFunctionFromEnum(function)); break;
		    case Aurora::FeatureControl::Blending:              glBlendFunc(GL_SRC_ALPHA, Utils::GLFunctionFromEnum(function)); break;
		    case Aurora::FeatureControl::StencilTesting:        AR_CORE_ASSERT(false); break;
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

	void RenderCommand::SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
	{
		glViewport(x, y, width, height);
	}

	void RenderCommand::DrawIndexed(const Ref<VertexArray>& vertexArray, uint32_t indexCount)
	{
		AR_PROFILE_FUNCTION();

		vertexArray->Bind();
		uint32_t count = indexCount == 0 ? vertexArray->GetIndexBuffer()->GetCount() : indexCount;
		glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, nullptr);
	}

}