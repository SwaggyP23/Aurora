#include "Aurorapch.h"
#include "RendererCaps.h"

#include <glad/glad.h>

namespace Aurora {

	static RendererProperties s_Props;

	void RendererCapabilities::Init()
	{
		AR_PROFILE_FUNCTION();

		s_Props.Vendor = (const char*)glGetString(GL_VENDOR);
		s_Props.Renderer = (const char*)glGetString(GL_RENDERER);
		s_Props.Version = (const char*)glGetString(GL_VERSION);
		s_Props.GLSLVersion = (const char*)glGetString(GL_SHADING_LANGUAGE_VERSION);

		glGetIntegerv(GL_MAX_SAMPLES, (int*)&(s_Props.MaxSamples));
		glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY, &(s_Props.MaxAnisotropy));
		glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, (int*)&s_Props.MaxTextureSlots);

		glGetIntegerv(GL_MAX_DRAW_BUFFERS, (int*)&s_Props.MaxDrawBuffers);
		glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, (int*)&s_Props.MaxColorAttachments);

		// Compute parameters
		glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 0, (int*)&s_Props.MaxWorkGroupCount[0]);
		glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 1, (int*)&s_Props.MaxWorkGroupCount[1]);
		glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 2, (int*)&s_Props.MaxWorkGroupCount[2]);
		glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 0, (int*)&s_Props.MaxWorkGroupSize[0]);
		glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 1, (int*)&s_Props.MaxWorkGroupSize[1]);
		glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 2, (int*)&s_Props.MaxWorkGroupSize[2]);
		glGetIntegerv(GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS, (int*)&s_Props.MaxWorkGroupInvocations);

		AR_CORE_INFO_TAG("Renderer", "Renderer Info:");
		AR_CORE_INFO_TAG("Renderer", "   --> Vendor: {0}", s_Props.Vendor);
		AR_CORE_INFO_TAG("Renderer", "   --> Renderer: {0}", s_Props.Renderer);
		AR_CORE_INFO_TAG("Renderer", "   --> OpenGL Version: {0}", s_Props.Version);
		AR_CORE_INFO_TAG("Renderer", "   --> GLSL Version: {0}", s_Props.GLSLVersion);
		AR_CORE_INFO_TAG("Renderer", "   --> Texture Slots: {0}", s_Props.MaxTextureSlots);
		AR_CORE_INFO_TAG("Renderer", "   --> Max Samples: {0}", s_Props.MaxSamples);
		AR_CORE_INFO_TAG("Renderer", "   --> Max Anisotropy: {0}", s_Props.MaxAnisotropy);
		AR_CORE_INFO_TAG("Renderer", "   --> Max Draw Buffers: {0}", s_Props.MaxDrawBuffers);
		AR_CORE_INFO_TAG("Renderer", "   --> Max Color Attachments: {0}", s_Props.MaxColorAttachments);

		// Compute paramters
		AR_CORE_INFO_TAG("Renderer", "Copmpute shaders properties");
		AR_CORE_INFO_TAG("Renderer", "   Max WorkGroup Count:");
		AR_CORE_INFO_TAG("Renderer", "      --> X: {0}, Y: {1}, Z: {2}", s_Props.MaxWorkGroupCount[0], s_Props.MaxWorkGroupCount[1], s_Props.MaxWorkGroupCount[2]);
		AR_CORE_INFO_TAG("Renderer", "   Max WorkGroup Size:");
		AR_CORE_INFO_TAG("Renderer", "      --> X: {0}, Y: {1}, Z: {2}", s_Props.MaxWorkGroupSize[0], s_Props.MaxWorkGroupSize[1], s_Props.MaxWorkGroupSize[2]);
		AR_CORE_INFO_TAG("Renderer", "   Max Invocations: {0}", s_Props.MaxWorkGroupInvocations);
	}

	void RendererCapabilities::ShutDown()
	{
	}

	const RendererProperties& RendererCapabilities::GetRendererProperties()
	{
		return s_Props;
	}

}