#pragma once

#include "OGLpch.h"
#include "Graphics/Shader.h"
#include "Graphics/Window.h"
#include "Graphics/Buffer.h"
#include "Graphics/VertexArray.h"
#include "Graphics/Texture.h"
#include "Utils/ImageLoader.h"
#include "Graphics/Camera.h"
#include "Events/ApplicationEvents.h" // This includes Events.h file

class Application
{
public:
	Application(const std::string& name);
	~Application();

	void Run();
	void onEvent(Event& e);
	
private:
	bool onWindowClose(WindowCloseEvent& e);
	bool onKeyPressed(KeyPressedEvent& e);
	//bool onKeyReleased(KeyReleasedEvent& e);
	//bool onMouseMove(MouseMovedEvent& e);

private:
	std::shared_ptr<Window> m_Window;
	std::shared_ptr<VertexArray> m_VertexArray;
	std::shared_ptr<VertexBuffer> m_VertexBuffer;
	std::shared_ptr<IndexBuffer> m_IndexBuffer;
	std::shared_ptr<Shader> m_Shader;
	std::shared_ptr<Camera> m_Camera;
	BufferLayout m_Layout;

	float m_DeltaTime = 0.0f;
	bool m_Running = true;
	bool m_IsRPressed = false;

	glm::vec4 color = glm::vec4(1.0f, 0.0f, 1.0f, 1.0f); // Initial clear color.
	glm::vec4 uniColor = glm::vec4(0.5f);

	std::vector<std::shared_ptr<Texture>> m_Textures;

	glm::vec3 m_CubePositions[10];
	glm::mat4 m_Projection;
};