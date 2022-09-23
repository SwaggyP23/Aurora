#pragma once

namespace Aurora {

	struct RendererProperties
	{
		std::string Vendor;
		std::string Renderer;
		std::string Version;
		std::string GLSLVersion;

		float MaxAnisotropy = 0.0f;
		uint32_t MaxSamples = 0;
		uint32_t MaxTextureSlots = 0;

		uint32_t MaxColorAttachments = 0;
		uint32_t MaxDrawBuffers = 0;

		uint32_t MaxWorkGroupInvocations = 0;
		std::array<uint32_t, 3> MaxWorkGroupCount;
		std::array<uint32_t, 3> MaxWorkGroupSize;
	};

	class RendererCapabilities
	{
	public:
		static void Init();
		static void ShutDown();

		// The meant usage of this is as a raw pointer and no need to create a ref or a scope from it!
		// RendererCapabilities has ownership of the RendererProperties and by that means it handles creation and deletion
		[[nodiscard]] static const RendererProperties& GetRendererProperties();

	};

}