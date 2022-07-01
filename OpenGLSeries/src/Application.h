#pragma once

#include "OGLpch.h"
#include "Graphics/EditorCamera.h"
//#include "Graphics/Camera.h"
#include "Graphics/Shader.h"
#include "Graphics/Window.h"
#include "Graphics/Buffer.h"
#include "Graphics/VertexArray.h"
#include "Graphics/Texture.h"
#include "Utils/ImageLoader.h"
#include "Core/TimeStep.h"
#include "Events/ApplicationEvents.h" // This includes Events.h file
#include "Events/MouseEvents.h"
#include "Layers/LayerStack.h"
#include "ImGui/ImGuiLayer.h"
#include "Input/Input.h"
#include "Graphics/RenderCommand.h"
#include "Graphics/Renderer.h"

class Application
{
public:
	Application(const std::string& name = "OpenGLSeries App");
	~Application();

	void Run();
	void onEvent(Event& e);
	
	inline bool& getVSync() { return m_VSync; }
	inline ImGuiLayer* getImGuiLayer() const { return m_ImGuiLayer; }
	inline Window& getWindow() const { return *m_Window; }
	inline static Application& getApp() { return *s_Instance; }

private:
	void pushLayer(Layer* layer);
	void pushOverlay(Layer* layer);
	bool onWindowClose(WindowCloseEvent& e);
	bool onWindowResize(WindowResizeEvent& e);
	bool onKeyPressed(KeyPressedEvent& e);

private:
	std::shared_ptr<Window> m_Window;

	std::shared_ptr<VertexArray> m_VertexArray;
	std::shared_ptr<VertexArray> m_GroundVertexArray;
	std::shared_ptr<VertexArray> m_LightVertexArray;
	std::shared_ptr<VertexArray> m_SphereVertexArray;

	std::shared_ptr<VertexBuffer> m_VertexBuffer;
	std::shared_ptr<VertexBuffer> m_GroundVertexBuffer;
	std::shared_ptr<VertexBuffer> m_SphereVertexBuffer;

	std::shared_ptr<IndexBuffer> m_IndexBuffer;
	std::shared_ptr<IndexBuffer> m_GroundIndexBuffer;
	std::shared_ptr<IndexBuffer> m_SphereIndexBuffer;

	std::shared_ptr<Shader> m_Shader;
	std::shared_ptr<Shader> m_LightShader;
	std::shared_ptr<Shader> m_GroundShader;
	std::shared_ptr<Shader> m_SphereShader;

	//std::shared_ptr<Camera> m_Camera; // This is the old camera
	std::shared_ptr<EditorCamera> m_Camera;
	ImGuiLayer* m_ImGuiLayer;
	LayerStack m_LayerStack;

	float m_LastFrame = 0.0f;

	bool m_Running = true;
	bool m_VSync = true;
	bool m_IsRPressed = false;

	glm::vec4 uniColor = glm::vec4(0.5f);

	std::vector<std::shared_ptr<Texture>> m_Textures;

	std::array<glm::vec3, 10> m_CubePositions;

	static Application* s_Instance;
};