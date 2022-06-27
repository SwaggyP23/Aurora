#include "OGLpch.h"
#include "Application.h"

Application* Application::s_Instance = nullptr;

Application::Application(const std::string& name)
{
	s_Instance = this;

	m_Window = std::make_shared<Window>(name, 1280, 720);
	m_Window->enable(GL_DEPTH_TEST);
	m_Window->SetVSync(false);
	m_Window->SetEventCallback(SET_EVENT_FN(Application::onEvent));


	glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);

	m_Camera = std::make_shared<Camera>(cameraPos);

	std::vector<char> errorMessage;

	//GLfloat vertices[4 * 6 * 9] = {
	//	-0.5f, -0.5f, -0.5f,    1.0f, 0.0f, 0.0f, 1.0f,   0.0f, 0.0f, // 0
	// 	-0.5f,  0.5f, -0.5f,    0.0f, 1.0f, 0.0f, 1.0f,   0.0f, 1.0f, // 1
	//	 0.5f,  0.5f, -0.5f,    0.0f, 0.0f, 1.0f, 1.0f,   1.0f, 1.0f, // 2
	//	 0.5f, -0.5f, -0.5f,	0.5f, 0.0f, 1.0f, 1.0f,   1.0f, 0.0f, // 3

	//    -0.5f, -0.5f, -0.5f,    1.0f, 0.0f, 0.0f, 1.0f,   0.0f, 0.0f, // 4
	//    -0.5f, -0.5f,  0.5f,    0.0f, 1.0f, 0.0f, 1.0f,   0.0f, 1.0f, // 5
	//    -0.5f,  0.5f, -0.5f,    0.0f, 0.0f, 1.0f, 1.0f,   1.0f, 1.0f, // 6
	//    -0.5f,  0.5f,  0.5f,	0.5f, 0.0f, 1.0f, 1.0f,   1.0f, 0.0f, // 7

	//    -0.5f, -0.5f,  0.5f,    1.0f, 0.0f, 0.0f, 1.0f,   0.0f, 0.0f, // 8
	//    -0.5f,  0.5f,  0.5f,    0.0f, 1.0f, 0.0f, 1.0f,   0.0f, 1.0f, // 9
	//     0.5f,  0.5f,  0.5f,    0.0f, 0.0f, 1.0f, 1.0f,   1.0f, 1.0f, // 10
	//     0.5f, -0.5f,  0.5f,	0.5f, 0.0f, 1.0f, 1.0f,   1.0f, 0.0f, // 11

	//	 0.5f, -0.5f, -0.5f,    1.0f, 0.0f, 0.0f, 1.0f,   0.0f, 0.0f, // 12
	//	 0.5f, -0.5f,  0.5f,    0.0f, 1.0f, 0.0f, 1.0f,   0.0f, 1.0f, // 13
	//	 0.5f,  0.5f, -0.5f,    0.0f, 0.0f, 1.0f, 1.0f,   1.0f, 1.0f, // 14
	//	 0.5f,  0.5f,  0.5f,	0.5f, 0.0f, 1.0f, 1.0f,   1.0f, 0.0f, // 15

	//    -0.5f, -0.5f, -0.5f,    1.0f, 0.0f, 0.0f, 1.0f,   0.0f, 0.0f, // 16
	//     0.5f, -0.5f, -0.5f,    0.0f, 1.0f, 0.0f, 1.0f,   0.0f, 1.0f, // 17
	//    -0.5f, -0.5f, -0.5f,    0.0f, 0.0f, 1.0f, 1.0f,   1.0f, 1.0f, // 18
	//     0.5f, -0.5f, -0.5f,	0.5f, 0.0f, 1.0f, 1.0f,   1.0f, 0.0f, // 19

	//	-0.5f, -0.5f,  0.5f,    1.0f, 0.0f, 0.0f, 1.0f,   0.0f, 0.0f, // 20
	//	 0.5f, -0.5f,  0.5f,    0.0f, 1.0f, 0.0f, 1.0f,   0.0f, 1.0f, // 21
	//	-0.5f, -0.5f,  0.5f,    0.0f, 0.0f, 1.0f, 1.0f,   1.0f, 1.0f, // 22
	//	 0.5f, -0.5f,  0.5f,	0.5f, 0.0f, 1.0f, 1.0f,   1.0f, 0.0f  // 23
	//};

	float vertices[] = {
		-0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
		 0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
		 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
		 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
		-0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, 0.0f,

		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
		 0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
		 0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
		 0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
		-0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,

		-0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
		-0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
		-0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

		 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
		 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
		 0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
		 0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
		 0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
		 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
		 0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
		 0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
		 0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,

		-0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
		 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
		 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
		 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
		-0.5f,  0.5f,  0.5f,  0.0f, 0.0f,
		-0.5f,  0.5f, -0.5f,  0.0f, 1.0f
	};

	//m_CubePositions[0] = glm::vec3(0.0f,  0.0f,  0.0f);
	//m_CubePositions[1] = glm::vec3(2.0f,  5.0f, -15.0f);
	//m_CubePositions[2] = glm::vec3(-1.5f, -2.2f, -2.5f);
	//m_CubePositions[3] = glm::vec3(-3.8f, -2.0f, -12.3f);
	//m_CubePositions[4] = glm::vec3(2.4f, -0.4f, -3.5f);
	//m_CubePositions[5] = glm::vec3(-1.7f,  3.0f, -7.5f);
	//m_CubePositions[6] = glm::vec3(1.3f, -2.0f, -2.5f);
	//m_CubePositions[7] = glm::vec3(1.5f,  2.0f, -2.5f);
	//m_CubePositions[8] = glm::vec3(1.5f,  0.2f, -1.5f);
	//m_CubePositions[9] = glm::vec3(-1.3f,  1.0f, -1.5f);

	m_CubePositions[0] = glm::vec3(0.0f, 0.0f, 0.0f);
	m_CubePositions[1] = glm::vec3(0.0f, 0.0f, -1.0f);
	m_CubePositions[2] = glm::vec3(0.0f, 0.0f, -2.0f);
	m_CubePositions[3] = glm::vec3(1.0f, 0.0f, 0.0f);
	m_CubePositions[4] = glm::vec3(-1.0f, 0.0f, 0.0f);
	m_CubePositions[5] = glm::vec3(-1.0f,  0.0f, -1.0f);
	m_CubePositions[6] = glm::vec3(-1.0f, 0.0f, -2.0f);
	m_CubePositions[7] = glm::vec3(1.0f,  0.0f, -1.0f);
	m_CubePositions[8] = glm::vec3(1.0f,  0.0f, -2.0f);
	m_CubePositions[9] = glm::vec3(0.0f,  1.0f, -1.0f);

	size_t indices[6] = { 0, 1, 2, 2, 3, 0 };

	m_VertexArray = std::make_shared<VertexArray>();

	m_Layout = {
		{ ShaderDataType::Float3, "a_Position" },
		{ ShaderDataType::Float2, "a_TexCoord" }
	};

	m_VertexBuffer = std::make_shared<VertexBuffer>(vertices, sizeof(vertices));
	m_VertexBuffer->bind();
	m_VertexBuffer->setLayout(m_Layout);
	m_VertexArray->addVertexBuffer(m_VertexBuffer);

	// the 2nd arg here should be sizeof(indices) / sizeof(uint32_t) but it gives warning
	m_IndexBuffer = std::make_shared<IndexBuffer>(indices, sizeof(indices) / sizeof(size_t));
	m_IndexBuffer->bind();
	m_VertexArray->setIndexBuffer(m_IndexBuffer);

	m_VertexBuffer->unBind();
	m_IndexBuffer->unBind();

	m_Shader = std::make_shared<Shader>("resources/shaders/Basic.shader");

	// Creating textures
	std::shared_ptr<Texture> text1 = std::make_shared<Texture>("resources/textures/minecraftGrass.png");
	text1->bind();
	text1->setTextureWrapping(GL_REPEAT);
	text1->setTextureFiltering(GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR);
	text1->loadTextureData(GL_RGBA, GL_RGBA);
	text1->unBind();

	std::shared_ptr<Texture> text2 = std::make_shared<Texture>("resources/textures/minecraftTNT.png");
	text2->bind();
	text2->setTextureWrapping(GL_REPEAT);
	text2->setTextureFiltering(GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR);
	text2->loadTextureData(GL_RGBA, GL_RGBA);
	text2->unBind();

	// Vector of textures for easily adding and binding textures in render loop	
	m_Textures.push_back(text1);
	m_Textures.push_back(text2);

	m_Shader->bind();
	m_Shader->setUniform1i("texture1", 0);
	m_Shader->setUniform1i("texture2", 1);

	m_ImGuiLayer = new ImGuiLayer();
	pushOverlay(m_ImGuiLayer);

	// Just some hard coded program start MVP matrices
	//glm::mat4 model = glm::rotate(glm::mat4(1.0f), glm::radians(45.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	//glm::mat4 view = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -3.0f));
	//m_Projection = glm::perspective(glm::radians(45.0f), 1024.0f / 576.0f, 0.1f, 100.0f);

	//m_Shader->setUniformMat4("ml_matrix", model);
	//m_Shader->setUniformMat4("vw_matrix", view);
	//m_Shader->setUniformMat4("pr_matrix", m_Projection);
}

Application::~Application()
{
}

void Application::pushLayer(Layer* layer)
{
	m_LayerStack.pushLayer(layer);
	layer->onAttach();
}

void Application::pushOverlay(Layer* layer)
{
	m_LayerStack.pushOverlay(layer);
	layer->onAttach();
}

void Application::onEvent(Event& e)
{
	EventDispatcher dispatcher(e);
	dispatcher.dispatch<WindowCloseEvent>(SET_EVENT_FN(Application::onWindowClose));
	dispatcher.dispatch<KeyPressedEvent>(SET_EVENT_FN(Application::onKeyPressed));
	dispatcher.dispatch<MouseMovedEvent>(SET_EVENT_FN(Application::onMouseMove));
	//dispatcher.dispatch<KeyReleasedEvent>(SET_EVENT_FN(Application::onKeyReleased));
	//LOG_INFO("{0}", e);

	for (auto it = m_LayerStack.rbegin(); it != m_LayerStack.rend(); ++it)
	{
		if (e.Handled)
			break;
		(*it)->onEvent(e);
	}
}

void Application::Run()
{
	//float fov = 45.0f;
	float rotation = 0.0f;
	glm::vec3 translation(0.0f, 0.0f, 0.0f);

	while(m_Running) // Render Loop.
	{
		float currentFrame = (float)(glfwGetTime());
		m_DeltaTime = currentFrame - m_LastFrame;
		m_LastFrame = currentFrame;

		glm::vec4 color = m_ImGuiLayer->getClearColor();
		m_Window->clear(color.r, color.g, color.b, 1.0f);

		for (Layer* layer : m_LayerStack)
			layer->onUpdate();

		m_ImGuiLayer->begin();
		for (Layer* layer : m_LayerStack)
			layer->onImGuiRender();
		m_ImGuiLayer->end();

		int i = 0;
		for (const auto& texture : m_Textures)
		{
			glActiveTexture(GL_TEXTURE0 + i);
			texture->bind();
			i++;
		}

		if (Input::isKeyPressed(GLFW_KEY_V)) {
			if (m_ImGuiLayer->getBlend() > 0.01f)
				m_ImGuiLayer->setBlend(m_ImGuiLayer->getBlend() - 0.01f);
		}
		if (Input::isKeyPressed(GLFW_KEY_B)) {
			if (m_ImGuiLayer->getBlend() < 0.990f)
				m_ImGuiLayer->setBlend(m_ImGuiLayer->getBlend() + 0.01f);
		}

		m_Shader->setUniform1f("blend", m_ImGuiLayer->getBlend());

		glm::mat4 view = m_Camera->GetViewMatrix();
		m_Shader->setUniformMat4("vw_matrix", view);

		m_Projection = glm::perspective(glm::radians(m_Camera->getZoom()), 1024.0f / 576.0f, 0.1f, 100.0f);
		m_Shader->setUniformMat4("pr_matrix", m_Projection);

		m_VertexArray->bind();
		for (unsigned int i = 0; i < 10; i++)
		{
			// calculate the model matrix for each object and pass it to shader before drawing
			glm::mat4 model = glm::mat4(1.0f); // make sure to initialize matrix to identity matrix first
			model = glm::translate(model, m_CubePositions[i] + translation);

			if (m_IsRPressed)
				model = glm::rotate(model, (float)glfwGetTime(), glm::vec3(1.0f, 0.3f, 0.5f));
			else
				model = glm::rotate(model, rotation, glm::vec3(1.0f, 0.3f, 0.5f));
			m_Shader->setUniformMat4("ml_matrix", model);

			glDrawArrays(GL_TRIANGLES, 0, 36);
		}
		m_VertexArray->unBind();

		m_Window->update();
	}
}

bool Application::onWindowClose(WindowCloseEvent& e)
{
	m_Running = false;
	return true;
}

bool Application::onKeyPressed(KeyPressedEvent& e)
{
	if (Input::isKeyPressed(GLFW_KEY_R))
		m_IsRPressed = !m_IsRPressed;
	if (Input::isKeyPressed(GLFW_KEY_W))
		m_Camera->ProcessKeyboard(FORWARD, m_DeltaTime);
	if (Input::isKeyPressed(GLFW_KEY_S))
		m_Camera->ProcessKeyboard( BACKWARD, m_DeltaTime);
	if (Input::isKeyPressed(GLFW_KEY_A))
		m_Camera->ProcessKeyboard(LEFT, m_DeltaTime);
	if (Input::isKeyPressed(GLFW_KEY_D))
		m_Camera->ProcessKeyboard(RIGHT, m_DeltaTime);
	if (Input::isKeyPressed(GLFW_KEY_Q))
		m_Camera->ProcessKeyboard(DOWNWARD, m_DeltaTime);
	if (Input::isKeyPressed(GLFW_KEY_E))
		m_Camera->ProcessKeyboard(UPWARD, m_DeltaTime);

	return true;
}

bool Application::onMouseMove(MouseMovedEvent& e)
{
	auto [x, y] = Input::getMousePosition();
	if (m_Camera->getFirstMouse()) {
		m_Camera->setLastx((float)(m_Window->getWidth() / 2));
		m_Camera->setLasty((float)(m_Window->getHeight() / 2));
		m_Camera->setFirstMouse(false);
	}

	float xoffset = x - m_Camera->getLastx();
	float yoffset = m_Camera->getLasty() - y;

	m_Camera->setLastx(x);
	m_Camera->setLasty(y);

	if(Input::isMouseButtonPressed(GLFW_MOUSE_BUTTON_RIGHT))
		m_Camera->ProcessMouseMovement(xoffset, yoffset);

	return true;
}