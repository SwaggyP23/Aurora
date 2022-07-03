#include "SandBoxLayer.h"

SandBoxLayer::SandBoxLayer()
	: Layer("SandBoxLayer"), m_Camera(CreateRef<EditorCamera>(45.0f, 16.0f / 9.0f, 0.1f, 1000.0f))
{
	float groundVertices[] = {
		-50.0f, -5.0f, -50.0f,   0.0f, 1.0f, 0.0f,   1.0f, 1.0f, 1.0f, 1.0f,   0.0f, 0.0f,
		 50.0f, -5.0f, -50.0f,   0.0f, 1.0f, 0.0f,   1.0f, 1.0f, 1.0f, 1.0f,   1.0f, 0.0f,
		 50.0f, -5.0f,  50.0f,   0.0f, 1.0f, 0.0f,   1.0f, 1.0f, 1.0f, 1.0f,   1.0f, 1.0f,
		-50.0f, -5.0f,  50.0f,   0.0f, 1.0f, 0.0f,   1.0f, 1.0f, 1.0f, 1.0f,   0.0f, 1.0f
	};

	uint32_t groundIndices[] = { 0, 1, 2, 2, 3, 0 };

	BufferLayout m_GroundLayout = {
		{ ShaderDataType::Float3, "a_Position" },
		{ ShaderDataType::Float3, "a_Normals" },
		{ ShaderDataType::Float4, "a_Color" },
		{ ShaderDataType::Float2, "a_TexCoord" }
	};

	// For ground...
	m_GroundVertexArray = VertexArray::Create();
	Ref<VertexBuffer> m_GroundVertexBuffer = VertexBuffer::Create(groundVertices, sizeof(groundVertices));
	m_GroundVertexBuffer->bind();
	m_GroundVertexBuffer->setLayout(m_GroundLayout);
	m_GroundVertexArray->addVertexBuffer(m_GroundVertexBuffer);
	Ref<IndexBuffer> m_GroundIndexBuffer = IndexBuffer::Create(groundIndices, sizeof(groundIndices) / sizeof(uint32_t));
	m_GroundIndexBuffer->bind();
	m_GroundVertexArray->setIndexBuffer(m_GroundIndexBuffer);

	m_GroundVertexBuffer->unBind();
	m_GroundIndexBuffer->unBind();

	// For sphere..
	std::vector<glm::vec3> positions;
	std::vector<glm::vec2> uv;
	std::vector<glm::vec3> normals;
	std::vector<glm::vec4> colors;
	std::vector<uint32_t> Sphereindices;

	const uint32_t X_SEGMENTS = 64;
	const uint32_t Y_SEGMENTS = 64;
	const float PI = 3.14159265359f;
	for (uint32_t x = 0; x <= X_SEGMENTS; ++x)
	{
		for (uint32_t y = 0; y <= Y_SEGMENTS; ++y)
		{
			float xSegment = (float)x / (float)X_SEGMENTS;
			float ySegment = (float)y / (float)Y_SEGMENTS;
			float xPos = std::cos(xSegment * 2.0f * PI) * std::sin(ySegment * PI);
			float yPos = std::cos(ySegment * PI);
			float zPos = std::sin(xSegment * 2.0f * PI) * std::sin(ySegment * PI);

			positions.push_back(glm::vec3(xPos, yPos, zPos));
			uv.push_back(glm::vec2(xSegment, ySegment));
			normals.push_back(glm::vec3(xPos, yPos, zPos));
			//colors.push_back(glm::vec4(xPos, yPos, zPos, 1.0f));
			colors.push_back(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
		}
	}

	bool oddRow = false;
	for (uint32_t y = 0; y < Y_SEGMENTS; ++y)
	{
		if (!oddRow) // even rows: y == 0, y == 2; and so on
		{
			for (uint32_t x = 0; x <= X_SEGMENTS; ++x)
			{
				Sphereindices.push_back(y * (X_SEGMENTS + 1) + x);
				Sphereindices.push_back((y + 1) * (X_SEGMENTS + 1) + x);
			}
		}
		else
		{
			for (int x = X_SEGMENTS; x >= 0; --x)
			{
				Sphereindices.push_back((y + 1) * (X_SEGMENTS + 1) + x);
				Sphereindices.push_back(y * (X_SEGMENTS + 1) + x);
			}
		}
		oddRow = !oddRow;
	}

	std::vector<float> data;
	for (uint32_t i = 0; i < positions.size(); ++i)
	{
		data.push_back(positions[i].x);
		data.push_back(positions[i].y);
		data.push_back(positions[i].z);
		if (normals.size() > 0)
		{
			data.push_back(normals[i].x);
			data.push_back(normals[i].y);
			data.push_back(normals[i].z);
		}
		if (uv.size() > 0)
		{
			data.push_back(uv[i].x);
			data.push_back(uv[i].y);
		}
		if (colors.size() > 0)
		{
			data.push_back(colors[i].r);
			data.push_back(colors[i].g);
			data.push_back(colors[i].b);
			data.push_back(colors[i].w);
		}
	}

	BufferLayout m_SphereLayout = {
			{ShaderDataType::Float3, "a_Pos" },
			{ShaderDataType::Float3, "a_Normal" },
			{ShaderDataType::Float2, "a_TexCoords" },
			{ShaderDataType::Float4, "a_Color" }
	};

	m_SphereVertexArray = VertexArray::Create();
	Ref<VertexBuffer> m_SphereVertexBuffer = VertexBuffer::Create(&data[0], data.size() * sizeof(float));
	m_SphereVertexBuffer->bind();
	m_SphereVertexBuffer->setLayout(m_SphereLayout);
	m_SphereVertexArray->addVertexBuffer(m_SphereVertexBuffer);

	Ref<IndexBuffer> m_SphereIndexBuffer = IndexBuffer::Create(&Sphereindices[0], Sphereindices.size());
	m_SphereIndexBuffer->bind();
	m_SphereVertexArray->setIndexBuffer(m_SphereIndexBuffer);

	m_SphereVertexBuffer->unBind();
	m_SphereIndexBuffer->unBind();

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
	//m_CubePositions[0] = glm::vec3(0.0f, 0.0f, 0.0f);
	//m_CubePositions[1] = glm::vec3(0.0f, 0.0f, -1.0f);
	//m_CubePositions[2] = glm::vec3(0.0f, 0.0f, -2.0f);
	//m_CubePositions[3] = glm::vec3(1.0f, 0.0f, 0.0f);
	//m_CubePositions[4] = glm::vec3(-1.0f, 0.0f, 0.0f);
	//m_CubePositions[5] = glm::vec3(-1.0f, 0.0f, -1.0f);
	//m_CubePositions[6] = glm::vec3(-1.0f, 0.0f, -2.0f);
	//m_CubePositions[7] = glm::vec3(1.0f, 0.0f, -1.0f);
	//m_CubePositions[8] = glm::vec3(1.0f, 0.0f, -2.0f);
	//m_CubePositions[9] = glm::vec3(0.0f, 1.0f, -1.0f);

	m_CubePositions[0] = glm::vec3(0.0f, 0.0f,  9.0f);
	m_CubePositions[1] = glm::vec3(0.0f, 0.0f,  7.0f);
	m_CubePositions[2] = glm::vec3(0.0f, 0.0f,  5.0f);
	m_CubePositions[3] = glm::vec3(0.0f, 0.0f,  3.0f);
	m_CubePositions[4] = glm::vec3(0.0f, 0.0f,  1.0f);
	m_CubePositions[5] = glm::vec3(0.0f, 0.0f, -1.0f);
	m_CubePositions[6] = glm::vec3(0.0f, 0.0f, -3.0f);
	m_CubePositions[7] = glm::vec3(0.0f, 0.0f, -5.0f);
	m_CubePositions[8] = glm::vec3(0.0f, 0.0f, -7.0f);
	m_CubePositions[9] = glm::vec3(0.0f, 0.0f, -9.0f);

	m_Shaders.Load("resources/shaders/Basic.shader");
	m_Shaders.Load("resources/shaders/Light.shader");
	m_Shaders.Load("resources/shaders/Ground.shader");
	m_Shaders.Load("resources/shaders/Sphere.shader");

	GLuint indices[6 * 6] = { 0, 1, 2, 2, 3, 0,
							  4, 5, 6, 6, 7, 4,
							  8, 9, 10, 10 ,11, 8,
							  12, 13, 14, 14, 15, 12,
							  16, 17, 18, 18, 19, 16,
							  20, 21, 22, 22, 23, 20 };

	BufferLayout m_Layout = {
		{ ShaderDataType::Float3, "a_Position" },
		{ ShaderDataType::Float3, "a_Normals" },
		{ ShaderDataType::Float4, "a_Color" },
		{ ShaderDataType::Float2, "a_TexCoord" }
	};

	// For main cube
	m_VertexArray = VertexArray::Create();

	Ref<VertexBuffer> m_VertexBuffer = VertexBuffer::Create(vertices, sizeof(vertices));
	m_VertexBuffer->bind();
	m_VertexBuffer->setLayout(m_Layout);
	m_VertexArray->addVertexBuffer(m_VertexBuffer);

	// the 2nd arg here should be sizeof(indices) / sizeof(uint32_t) but it gives warning
	Ref<IndexBuffer> m_IndexBuffer = IndexBuffer::Create(indices, sizeof(indices) / sizeof(GLuint));
	m_IndexBuffer->bind();
	m_VertexArray->setIndexBuffer(m_IndexBuffer);

	m_VertexBuffer->unBind();
	m_IndexBuffer->unBind();

	// for light source
	m_LightVertexArray = VertexArray::Create();
	m_VertexBuffer->bind();
	m_LightVertexArray->addVertexBuffer(m_VertexBuffer);
	m_IndexBuffer->bind();
	m_LightVertexArray->setIndexBuffer(m_IndexBuffer);

	m_VertexBuffer->unBind();
	m_IndexBuffer->unBind();

	// Creating textures
	Ref<Texture> text1 = Texture::Create("resources/textures/NewYork.png");
	text1->bind();
	text1->flipTextureVertically(true);
	text1->setTextureWrapping(GL_REPEAT);
	text1->setTextureFiltering(GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR);
	text1->loadTextureData(GL_RGB, GL_RGB);
	text1->unBind();

	Ref<Texture> text2 = Texture::Create("resources/textures/Pepsi.png");
	text2->bind();
	text2->flipTextureVertically(true);
	text2->setTextureWrapping(GL_REPEAT);
	text2->setTextureFiltering(GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR);
	text2->loadTextureData(GL_RGB, GL_RGB);
	text2->unBind();

	Ref<Texture> text3 = Texture::Create("resources/textures/checkerboard.png");
	text3->bind();
	text2->flipTextureVertically(true);
	text3->setTextureWrapping(GL_REPEAT);
	text3->setTextureFiltering(GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR);
	text3->loadTextureData(GL_RGBA, GL_RGBA);
	text3->unBind();

	Ref<Texture> text4 = Texture::Create("resources/textures/map.jpg");
	text4->bind();
	text4->flipTextureVertically(true);
	text4->setTextureWrapping(GL_REPEAT);
	text4->setTextureFiltering(GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR);
	text4->loadTextureData(GL_RGB, GL_RGB);
	text4->unBind();

	// Vector of textures for easily adding and binding textures in render loop	
	m_Textures.push_back(text1);
	m_Textures.push_back(text2);
	m_Textures.push_back(text3);
	m_Textures.push_back(text4);

	m_Shaders.Get("Basic")->bind();
	m_Shaders.Get("Basic")->setUniform1i("texture1", 0);
	m_Shaders.Get("Basic")->setUniform1i("texture2", 1);
	m_Shaders.Get("Basic")->unBind();

	m_Shaders.Get("Ground")->bind();
	m_Shaders.Get("Ground")->setUniform1i("Groundtexture1", 2);
	m_Shaders.Get("Ground")->unBind();

	m_Shaders.Get("Sphere")->bind();
	m_Shaders.Get("Sphere")->setUniform1i("SphereTexture1", 3);
	m_Shaders.Get("Sphere")->unBind();
}

void SandBoxLayer::onAttach()
{
}

void SandBoxLayer::onDetach()
{
}

void SandBoxLayer::onUpdate(/*should take in timestep*/)
{
	RenderCommand::setClearColor(m_Color);
	RenderCommand::Clear();

	RenderCommand::ActivateTextures(m_Textures);

	Renderer::BeginScene(m_Camera);

	m_Shaders.Get("Sphere")->bind();
	m_Shaders.Get("Sphere")->setUniform4f("lightColor", m_LightColor);
	m_Shaders.Get("Sphere")->setUniform1f("blend", m_Blend);
	m_Shaders.Get("Sphere")->setUniform1f("ambientStrength", m_AmbLight);
	m_Shaders.Get("Sphere")->setUniform3f("src_pos", m_LightTranslations);
	m_Shaders.Get("Sphere")->setUniform3f("view_pos", m_Camera->GetPosition());
	m_Shaders.Get("Sphere")->setUniform4f("src_color", m_LightColor);
	m_Shaders.Get("Sphere")->setUniform4f("un_color", m_UniColor);
	glm::mat4 model(1.0f);
	model = glm::translate(model, m_SphereTransalations);
	glm::vec3 angle = m_SphereRotations;
	glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), glm::radians(angle.x), { 1.0f, 0.0f, 0.0f })
		* glm::rotate(glm::mat4(1.0f), glm::radians(angle.y), { 0.0f, 1.0f, 0.0f })
		* glm::rotate(glm::mat4(1.0f), glm::radians(180.0f + angle.z), { 0.0f, 0.0f, 1.0f });
	model *= rotation;

	model = glm::scale(model, m_SphereScales);

	Renderer::DrawSphere(m_Shaders.Get("Sphere"), model, m_SphereVertexArray);

	m_Shaders.Get("Basic")->bind();
	m_Shaders.Get("Basic")->setUniform4f("lightColor", m_LightColor);
	m_Shaders.Get("Basic")->setUniform1f("blend", m_Blend);
	m_Shaders.Get("Basic")->setUniform1f("ambientStrength", m_AmbLight);
	m_Shaders.Get("Basic")->setUniform3f("src_pos", m_LightTranslations);
	m_Shaders.Get("Basic")->setUniform3f("view_pos", m_Camera->GetPosition());
	m_Shaders.Get("Basic")->setUniform4f("src_color", m_LightColor);
	m_Shaders.Get("Basic")->setUniform4f("un_color", m_UniColor);
	for (uint32_t i = 0; i < m_CubePositions.size(); i++)
	{
		// calculate the model matrix for each object and pass it to shader before drawing
		model = glm::mat4(1.0f); // make sure to initialize matrix to identity matrix first
		model = glm::translate(model, m_CubePositions[i] + m_Transalations);

		if (m_IsRPressed)
			model = glm::rotate(model, (float)glfwGetTime(), glm::vec3(1.0f, 0.3f, 0.5f));
		else
			model = glm::rotate(model, m_Rotation, glm::vec3(1.0f, 0.3f, 0.5f));

		model = glm::scale(model, m_Scales);

		Renderer::DrawQuad(m_Shaders.Get("Basic"), model, m_VertexArray);
	}

	m_Shaders.Get("Ground")->bind();
	m_Shaders.Get("Ground")->setUniform4f("lightColor", m_LightColor);
	m_Shaders.Get("Ground")->setUniform1f("blend", m_Blend);
	m_Shaders.Get("Ground")->setUniform1f("ambientStrength", m_AmbLight);
	m_Shaders.Get("Ground")->setUniform3f("src_pos", m_LightTranslations);
	m_Shaders.Get("Ground")->setUniform3f("view_pos", m_Camera->GetPosition());
	m_Shaders.Get("Ground")->setUniform4f("src_color", m_LightColor);
	m_Shaders.Get("Ground")->setUniform4f("un_color", m_UniColor);
	model = glm::translate(glm::mat4(1.0f), m_GroundTranslations);
	model = glm::scale(model, m_GroundScales);

	Renderer::DrawQuad(m_Shaders.Get("Ground"), model, m_GroundVertexArray);

	m_Shaders.Get("Light")->bind();
	m_Shaders.Get("Light")->setUniform4f("lightColor", m_LightColor);
	m_Shaders.Get("Light")->setUniform1f("blend", m_Blend);
	m_Shaders.Get("Light")->setUniform1f("ambientStrength", m_AmbLight);
	m_Shaders.Get("Light")->setUniform3f("src_pos", m_LightTranslations);
	m_Shaders.Get("Light")->setUniform3f("view_pos", m_Camera->GetPosition());
	m_Shaders.Get("Light")->setUniform4f("src_color", m_LightColor);
	m_Shaders.Get("Light")->setUniform4f("un_color", m_UniColor);
	model = glm::translate(glm::mat4(1.0f), m_LightTranslations);
	model = glm::scale(model, m_LightScales);

	Renderer::DrawQuad(m_Shaders.Get("Light"), model, m_LightVertexArray);

	Renderer::EndScene();

	m_Camera->OnUpdate();
}

void SandBoxLayer::onEvent(Event& e)
{
	m_Camera->OnEvent(e);

	if (Input::isKeyPressed(GLFW_KEY_R))
		m_IsRPressed = !m_IsRPressed;
}

void SandBoxLayer::onImGuiRender()
{
	Application& app = Application::getApp();

	ImGui::Begin("Editing Panel");
	if (ImGui::CollapsingHeader("Cube")) {
		//ImGui::Begin("Editing");
		ImGui::ColorEdit3("Uniform Color", (float*)&m_UniColor);
		ImGui::SliderFloat3("Translation", (float*)&m_Transalations, 0.0f, 5.0f);
		ImGui::SliderFloat("Rotation", (float*)&m_Rotation, 0.0f, 2.0f);
		ImGui::SliderFloat3("Scale", (float*)&m_Scales, 0.0f, 3.0f);
		ImGui::SliderFloat("Blend", &m_Blend, 0.0f, 1.0f);
		//ImGui::End();
	}

	ImGui::Separator();

	if (ImGui::CollapsingHeader("Light Source")) {
		//ImGui::Begin("Editing");
		ImGui::ColorEdit3("Light Color", (float*)&m_LightColor);
		ImGui::SliderFloat3("Light Translation", (float*)&m_LightTranslations, -30.0f, 135.0f);
		ImGui::SliderFloat3("Light Scale", (float*)&m_LightScales, 0.0f, 3.0f);
		//ImGui::End();
	}

	ImGui::Separator();

	if (ImGui::CollapsingHeader("Ball")) {
		//ImGui::Begin();
		ImGui::SliderFloat3("Sphere Translation", (float*)&m_SphereTransalations, -50.0f, 50.0f);
		ImGui::SliderFloat3("Sphere Rotations", (float*)&m_SphereRotations, 0.0f, 360.0f);
		ImGui::SliderFloat3("Sphere Scale", (float*)&m_SphereScales, 0.0f, 30.0f);
		//ImGui::End();
	}

	ImGui::Separator();

	if (ImGui::CollapsingHeader("Ground")) {
		//ImGui::Begin("Editing");
		ImGui::SliderFloat3("Ground Translation", (float*)&m_GroundTranslations, -5.0f, 5.0f);
		ImGui::SliderFloat3("Ground Scale", (float*)&m_GroundScales, 0.0f, 3.0f);
		//ImGui::End();
	}

	ImGui::Separator();

	ImGui::ColorEdit3("Clear Color", (float*)&m_Color);
	ImGui::SliderFloat("Ambient Light", &m_AmbLight, 0.0f, 1.0f);
	ImGui::Text("Framerate: %.f", ImGui::GetIO().Framerate);
	ImGui::Checkbox("V Sync ", &(app.getVSync()));

	ImGui::End();
}