#include "Aurorapch.h"
#include "RendererPorperties.h"

#include <glad/glad.h>

namespace Aurora {

	Properties* RendererProperties::s_Props;

	void RendererProperties::Init()
	{
		AR_PROFILE_FUNCTION();

		s_Props = new Properties();

		s_Props->Vendor = (const char*)glGetString(GL_VENDOR);
		s_Props->Renderer = (const char*)glGetString(GL_RENDERER);
		s_Props->Version = (const char*)glGetString(GL_VERSION);
		s_Props->GLSLVersion = (const char*)glGetString(GL_SHADING_LANGUAGE_VERSION);
		glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, (int*)&s_Props->TextureSlots);

		AR_CORE_LOG_INFO("OpenGL Info:");
		AR_CORE_LOG_INFO("   --> Vendor: {0}", s_Props->Vendor);
		AR_CORE_LOG_INFO("   --> Renderer: {0}", s_Props->Renderer);
		AR_CORE_LOG_INFO("   --> OpenGL Version: {0}", s_Props->Version);
		AR_CORE_LOG_INFO("   --> GLSL Version: {0}", s_Props->GLSLVersion);
		AR_CORE_LOG_INFO("   --> Texture Slots: {0}", s_Props->TextureSlots);
	}

	void RendererProperties::ShutDown()
	{
		delete s_Props;
	}

	const Properties* RendererProperties::GetRendererProperties()
	{
		return s_Props;
	}

}