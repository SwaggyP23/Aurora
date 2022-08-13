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

		glGetIntegerv(GL_MAX_SAMPLES, (int*)&(s_Props->MaxSamples));
		//glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY, &(s_Props->MaxAnisotropy));
		glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, (int*)&s_Props->MaxTextureSlots);

		AR_CORE_INFO_TAG("Renderer", "Renderer Info:");
		AR_CORE_INFO_TAG("Renderer", "   --> Vendor: {0}", s_Props->Vendor);
		AR_CORE_INFO_TAG("Renderer", "   --> Renderer: {0}", s_Props->Renderer);
		AR_CORE_INFO_TAG("Renderer", "   --> OpenGL Version: {0}", s_Props->Version);
		AR_CORE_INFO_TAG("Renderer", "   --> GLSL Version: {0}", s_Props->GLSLVersion);
		AR_CORE_INFO_TAG("Renderer", "   --> Texture Slots: {0}", s_Props->MaxTextureSlots);
		AR_CORE_INFO_TAG("Renderer", "   --> Max Samples: {0}", s_Props->MaxSamples);
		//AR_CORE_INFO_TAG("Renderer", "   --> Max Anisotropy: {0}", s_Props->MaxAnisotropy);
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