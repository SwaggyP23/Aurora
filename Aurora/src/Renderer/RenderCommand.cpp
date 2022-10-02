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
			    case RenderFlags::None:            return GL_NONE;
			    case RenderFlags::Fill:            return GL_FILL;
			    case RenderFlags::WireFrame:       return GL_LINE;
			    case RenderFlags::Vertices:        return GL_POINT;
			}

			AR_CORE_ASSERT(false, "Unknown Render Flag!");
			return 0;
		}

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

	void RenderCommand::Init()
	{
		// These are the defualt types of function and enables that the editor comes with right out of the box, things could be added or removed using the settings panel
		AR_PROFILE_FUNCTION();

		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LESS);

		glEnable(GL_MULTISAMPLE);
		glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
		// glEnable(GL_FRAMEBUFFER_SRGB);
		// Different color space

		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
		glFrontFace(GL_CCW);

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

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