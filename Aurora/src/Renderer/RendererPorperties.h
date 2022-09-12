#pragma once

namespace Aurora {

	struct Properties
	{
		std::string Vendor;
		std::string Renderer;
		std::string Version;
		std::string GLSLVersion;

		float MaxAnisotropy = 0.0f;
		uint32_t MaxSamples = 0;
		uint32_t MaxTextureSlots = 0;
		uint32_t MaxDrawBuffers = 0;
	};

	class RendererProperties
	{
	public:
		static void Init();
		static void ShutDown();

		static const Properties* GetRendererProperties();

	private:
		static Properties* s_Props;
	};

}