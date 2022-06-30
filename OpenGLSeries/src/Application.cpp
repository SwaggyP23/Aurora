#include "OGLpch.h"
#include "Application.h"

Application* Application::s_Instance = nullptr;

Application::Application(const std::string& name)
{
	s_Instance = this;

	m_Window = std::make_shared<Window>(name, 1280, 720);
	m_Window->enable(GL_DEPTH_TEST);
	m_Window->SetVSync(true);
	m_Window->SetEventCallback(SET_EVENT_FN(Application::onEvent));

	m_Camera = std::make_shared<Hazel::EditorCamera>(45.0f, 16.0f / 9.0f, 0.1f, 1000.0f);

	std::vector<char> errorMessage;

	float groundVertices[] = {
		-50.0f, -5.0f, -50.0f,   0.0f, 1.0f, 0.0f,   1.0f, 1.0f, 1.0f, 1.0f,   0.0f, 0.0f,
		 50.0f, -5.0f, -50.0f,   0.0f, 1.0f, 0.0f,   1.0f, 1.0f, 1.0f, 1.0f,   1.0f, 0.0f,
		 50.0f, -5.0f,  50.0f,   0.0f, 1.0f, 0.0f,   1.0f, 1.0f, 1.0f, 1.0f,   1.0f, 1.0f,
		-50.0f, -5.0f,  50.0f,   0.0f, 1.0f, 0.0f,   1.0f, 1.0f, 1.0f, 1.0f,   0.0f, 1.0f
	};

	uint32_t groundIndices[] = { 0, 1, 2, 2, 3, 0 };

	m_GroundLayout = {
		{ ShaderDataType::Float3, "a_Position" },
		{ ShaderDataType::Float3, "a_Normals" },
		{ ShaderDataType::Float4, "a_Color" },
		{ ShaderDataType::Float2, "a_TexCoord" }
	};

	// For ground...
	m_GroundVertexArray = std::make_shared<VertexArray>();
	m_GroundVertexBuffer = std::make_shared<VertexBuffer>(groundVertices, sizeof(groundVertices));
	m_GroundVertexBuffer->bind();
	m_GroundVertexBuffer->setLayout(m_GroundLayout);
	m_GroundVertexArray->addVertexBuffer(m_GroundVertexBuffer);
	m_GroundIndexBuffer = std::make_shared<IndexBuffer>(groundIndices, sizeof(groundIndices) / sizeof(uint32_t));
	m_GroundIndexBuffer->bind();
	m_GroundVertexArray->setIndexBuffer(m_GroundIndexBuffer);

	m_GroundVertexBuffer->unBind();
	m_GroundIndexBuffer->unBind();

	float vertices[] = {
		-0.5f, -0.5f, -0.5f,  0.0f, 0.0f, -1.0f,     1.0f, 1.0f, 1.0f, 1.0f,     0.0f, 0.0f,
		 0.5f, -0.5f, -0.5f,  0.0f, 0.0f, -1.0f,     1.0f, 1.0f, 1.0f, 1.0f,     1.0f, 0.0f,
		 0.5f,  0.5f, -0.5f,  0.0f, 0.0f, -1.0f,     1.0f, 1.0f, 1.0f, 1.0f,     1.0f, 1.0f,
		-0.5f,  0.5f, -0.5f,  0.0f, 0.0f, -1.0f,     1.0f, 1.0f, 1.0f, 1.0f,     0.0f, 1.0f,
														   
		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,  1.0f,     1.0f, 1.0f, 1.0f, 1.0f,     0.0f, 0.0f,
		 0.5f, -0.5f,  0.5f,  0.0f, 0.0f,  1.0f,     1.0f, 1.0f, 1.0f, 1.0f,     1.0f, 0.0f,
		 0.5f,  0.5f,  0.5f,  0.0f, 0.0f,  1.0f,     1.0f, 1.0f, 1.0f, 1.0f,     1.0f, 1.0f,
		-0.5f,  0.5f,  0.5f,  0.0f, 0.0f,  1.0f,     1.0f, 1.0f, 1.0f, 1.0f,     0.0f, 1.0f,

		-0.5f,  0.5f,  0.5f,  -1.0f, 0.0f, 0.0f,     1.0f, 1.0f, 1.0f, 1.0f,     1.0f, 0.0f,
		-0.5f,  0.5f, -0.5f,  -1.0f, 0.0f, 0.0f,     1.0f, 1.0f, 1.0f, 1.0f,     1.0f, 1.0f,
		-0.5f, -0.5f, -0.5f,  -1.0f, 0.0f, 0.0f,     1.0f, 1.0f, 1.0f, 1.0f,     0.0f, 1.0f,
		-0.5f, -0.5f,  0.5f,  -1.0f, 0.0f, 0.0f,     1.0f, 1.0f, 1.0f, 1.0f,     0.0f, 0.0f,
							   
		 0.5f,  0.5f,  0.5f,   1.0f, 0.0f, 0.0f,     1.0f, 1.0f, 1.0f, 1.0f,     1.0f, 0.0f,
		 0.5f,  0.5f, -0.5f,   1.0f, 0.0f, 0.0f,     1.0f, 1.0f, 1.0f, 1.0f,     1.0f, 1.0f,
		 0.5f, -0.5f, -0.5f,   1.0f, 0.0f, 0.0f,     1.0f, 1.0f, 1.0f, 1.0f,     0.0f, 1.0f,
		 0.5f, -0.5f,  0.5f,   1.0f, 0.0f, 0.0f,     1.0f, 1.0f, 1.0f, 1.0f,     0.0f, 0.0f,
							   
		-0.5f, -0.5f, -0.5f,  0.0f, -1.0f, 0.0f,     1.0f, 1.0f, 1.0f, 1.0f,     0.0f, 1.0f,
		 0.5f, -0.5f, -0.5f,  0.0f, -1.0f, 0.0f,     1.0f, 1.0f, 1.0f, 1.0f,     1.0f, 1.0f,
		 0.5f, -0.5f,  0.5f,  0.0f, -1.0f, 0.0f,     1.0f, 1.0f, 1.0f, 1.0f,     1.0f, 0.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, -1.0f, 0.0f,     1.0f, 1.0f, 1.0f, 1.0f,     0.0f, 0.0f,
							   
		-0.5f,  0.5f, -0.5f,  0.0f,  1.0f, 0.0f,      1.0f, 1.0f, 1.0f, 1.0f,    0.0f, 1.0f,
		 0.5f,  0.5f, -0.5f,  0.0f,  1.0f, 0.0f,      1.0f, 1.0f, 1.0f, 1.0f,    1.0f, 1.0f,
		 0.5f,  0.5f,  0.5f,  0.0f,  1.0f, 0.0f,      1.0f, 1.0f, 1.0f, 1.0f,    1.0f, 0.0f,
		-0.5f,  0.5f,  0.5f,  0.0f,  1.0f, 0.0f,      1.0f, 1.0f, 1.0f, 1.0f,    0.0f, 0.0f
	};

	//m_CubePositions[0] = glm::vec3(0.0f, 0.0f, 0.0f);
	//m_CubePositions[1] = glm::vec3(2.0f, 5.0f, -15.0f);
	//m_CubePositions[2] = glm::vec3(-1.5f, -2.2f, -2.5f);
	//m_CubePositions[3] = glm::vec3(-3.8f, -2.0f, -12.3f);
	//m_CubePositions[4] = glm::vec3(2.4f, -0.4f, -3.5f);
	//m_CubePositions[5] = glm::vec3(-1.7f, 3.0f, -7.5f);
	//m_CubePositions[6] = glm::vec3(1.3f, -2.0f, -2.5f);
	//m_CubePositions[7] = glm::vec3(1.5f, 2.0f, -2.5f);
	//m_CubePositions[8] = glm::vec3(1.5f, 0.2f, -1.5f);
	//m_CubePositions[9] = glm::vec3(-1.3f, 1.0f, -1.5f);
	// Currently i dont want to use any of these since i only want to draw the first box and then the light src
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

	m_Shader = std::make_shared<Shader>("resources/shaders/Basic.shader");
	m_LightShader = std::make_shared<Shader>("resources/shaders/Light.shader");
	m_GroundShader = std::make_shared<Shader>("resources/shaders/Ground.shader");

	GLuint indices[6 * 6] = { 0, 1, 2, 2, 3, 0,
							  4, 5, 6, 6, 7, 4,
							  8, 9, 10, 10 ,11, 8,
							  12, 13, 14, 14, 15, 12,
							  16, 17, 18, 18, 19, 16,
							  20, 21, 22, 22, 23, 20 };

	m_Layout = {
		{ ShaderDataType::Float3, "a_Position" },
		{ ShaderDataType::Float3, "a_Normals" },
		{ ShaderDataType::Float4, "a_Color" },
		{ ShaderDataType::Float2, "a_TexCoord" }
	};

	// For main cube
	m_VertexArray = std::make_shared<VertexArray>();

	m_VertexBuffer = std::make_shared<VertexBuffer>(vertices, sizeof(vertices));
	m_VertexBuffer->bind();
	m_VertexBuffer->setLayout(m_Layout);
	m_VertexArray->addVertexBuffer(m_VertexBuffer);

	// the 2nd arg here should be sizeof(indices) / sizeof(uint32_t) but it gives warning
	m_IndexBuffer = std::make_shared<IndexBuffer>(indices, sizeof(indices) / sizeof(GLuint));
	m_IndexBuffer->bind();
	m_VertexArray->setIndexBuffer(m_IndexBuffer);

	m_VertexBuffer->unBind();
	m_IndexBuffer->unBind();

	// for light source
	m_LightVertexArray = std::make_shared<VertexArray>();
	m_VertexBuffer->bind();
	m_LightVertexArray->addVertexBuffer(m_VertexBuffer);
	m_IndexBuffer->bind();
	m_LightVertexArray->setIndexBuffer(m_IndexBuffer);

	m_VertexBuffer->unBind();
	m_IndexBuffer->unBind();

	// Creating textures
	std::shared_ptr<Texture> text1 = std::make_shared<Texture>("resources/textures/ice.png");
	text1->bind();
	text1->setTextureWrapping(GL_REPEAT);
	text1->setTextureFiltering(GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR);
	text1->loadTextureData(GL_RGBA, GL_RGBA);
	text1->unBind();

	std::shared_ptr<Texture> text2 = std::make_shared<Texture>("resources/textures/Qiyana.jpg");
	text2->bind();
	text2->setTextureWrapping(GL_REPEAT);
	text2->setTextureFiltering(GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR);
	text2->loadTextureData(GL_RGB, GL_RGB);
	text2->unBind();

	std::shared_ptr<Texture> text3 = std::make_shared<Texture>("resources/textures/checkerboard.png");
	text3->bind();
	text3->setTextureWrapping(GL_REPEAT);
	text3->setTextureFiltering(GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR);
	text3->loadTextureData(GL_RGBA, GL_RGBA);
	text3->unBind();

	// Vector of textures for easily adding and binding textures in render loop	
	m_Textures.push_back(text1);
	m_Textures.push_back(text2);
	m_Textures.push_back(text3);

	m_Shader->bind();
	m_Shader->setUniform1i("texture1", 0);
	m_Shader->setUniform1i("texture2", 1);
	m_Shader->unBind();

	m_GroundShader->bind();
	m_GroundShader->setUniform1i("Groundtexture1", 2);
	m_GroundShader->unBind();

	m_ImGuiLayer = new ImGuiLayer();
	pushOverlay(m_ImGuiLayer);
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
	m_Camera->OnEvent(e);
	m_Window->SetVSync(m_VSync);	

	EventDispatcher dispatcher(e);
	dispatcher.dispatch<WindowCloseEvent>(SET_EVENT_FN(Application::onWindowClose));
	//dispatcher.dispatch<WindowResizeEvent>(SET_EVENT_FN(Application::onWindowResize));
	dispatcher.dispatch<KeyPressedEvent>(SET_EVENT_FN(Application::onKeyPressed));
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
	glm::mat4 model;

	while(m_Running) // Render Loop.
	{
		float currentFrame = (float)(glfwGetTime());
		TimeStep time = currentFrame - m_LastFrame;
		m_LastFrame = currentFrame;

		glm::vec4 color = m_ImGuiLayer->getClearColor();
		m_Window->clear(color.r, color.g, color.b, 1.0f);

		int i = 0;
		for (const auto& texture : m_Textures)
		{
			glActiveTexture(GL_TEXTURE0 + i);
			texture->bind();
			i++;
		}

		m_Shader->bind();
		m_Shader->setUniform1f("blend", m_ImGuiLayer->getBlend());
		m_Shader->setUniform1f("ambientStrength", m_ImGuiLayer->getAmbLight() - 0.3f);
		m_Shader->setUniform3f("src_pos", m_ImGuiLayer->getLightTranslations());
		m_Shader->setUniform3f("view_pos", m_Camera->GetPosition());
		m_Shader->setUniform4f("src_color", m_ImGuiLayer->getLightColor());
		m_Shader->setUniform4f("un_color", m_ImGuiLayer->getUniColor());

		glm::mat4 view = m_Camera->GetViewMatrix();
		m_Shader->setUniformMat4("vw_matrix", view);

		glm::mat4 projection = m_Camera->GetProjection();
		m_Shader->setUniformMat4("pr_matrix", projection);

		m_VertexArray->bind();
		for (unsigned int i = 0; i < m_CubePositions.size(); i++)
		{
			// calculate the model matrix for each object and pass it to shader before drawing
			model = glm::mat4(1.0f); // make sure to initialize matrix to identity matrix first
			model = glm::translate(model, m_CubePositions[i] + m_ImGuiLayer->getTransalations());

			if (m_IsRPressed)
				model = glm::rotate(model, (float)glfwGetTime(), glm::vec3(1.0f, 0.3f, 0.5f));
			else
				model = glm::rotate(model, m_ImGuiLayer->getRotation(), glm::vec3(1.0f, 0.3f, 0.5f));

			model = glm::scale(model, m_ImGuiLayer->getScales());

			m_Shader->setUniformMat4("ml_matrix", model);
			m_Shader->setUniformMat3("normalMatrix", glm::transpose(glm::inverse(model)));

			glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
		}
		//m_VertexArray->unBind(); // Not necessary
		
		m_GroundShader->bind();
		m_GroundShader->setUniform1f("ambientStrength", m_ImGuiLayer->getAmbLight() - 0.5f);
		m_GroundShader->setUniform3f("src_pos", m_ImGuiLayer->getLightTranslations());
		m_GroundShader->setUniform3f("view_pos", m_Camera->GetPosition());
		m_GroundShader->setUniform4f("src_color", m_ImGuiLayer->getLightColor());
		m_GroundShader->setUniformMat4("pr_matrix", projection);
		m_GroundShader->setUniformMat4("vw_matrix", view);
		model = glm::translate(glm::mat4(1.0f), m_ImGuiLayer->getGroundTranslations());
		model = glm::scale(model, m_ImGuiLayer->getGroundScales());
		m_GroundShader->setUniformMat4("ml_matrix", model);
		m_GroundShader->setUniformMat3("normalMatrix", glm::transpose(glm::inverse(model)));
		m_GroundVertexArray->bind();
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

		m_LightShader->bind();
		m_LightShader->setUniform1f("ambientStrength", m_ImGuiLayer->getAmbLight());
		m_LightShader->setUniform4f("lightColor", m_ImGuiLayer->getLightColor());
		m_LightShader->setUniformMat4("pr_matrix", projection);
		m_LightShader->setUniformMat4("vw_matrix", view);
		model = glm::translate(glm::mat4(1.0f), m_ImGuiLayer->getLightTranslations());
		model = glm::scale(model, m_ImGuiLayer->getLightScales());
		m_LightShader->setUniformMat4("ml_matrix", model);

		m_LightVertexArray->bind();
		glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
		//m_LightVertexArray->unBind();

		for (Layer* layer : m_LayerStack)
			layer->onUpdate();

		m_ImGuiLayer->begin();
		for (Layer* layer : m_LayerStack)
			layer->onImGuiRender();
		m_ImGuiLayer->end();

		m_Camera->OnUpdate();
		m_Window->update();
	}
}

bool Application::onWindowResize(WindowResizeEvent& e)
{
	glViewport(0, 0, e.getWidth(), e.getHeight());

	return true;
}

bool Application::onWindowClose(WindowCloseEvent& e)
{
	m_Running = false;
	return true;
}

bool Application::onKeyPressed(KeyPressedEvent& e)
{
	if (Input::isKeyPressed(GLFW_KEY_ESCAPE))
		m_Running = false;
	if (Input::isKeyPressed(GLFW_KEY_R))
		m_IsRPressed = !m_IsRPressed;

	return true;
}