#pragma once

struct Properties
{
	const char* Vendor = nullptr;
	const char* Renderer = nullptr;
	const char* Version = nullptr;
	const char* GLSLVersion = nullptr;
	uint32_t TextureSlots = 0;
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