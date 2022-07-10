#include "SandBoxLayer.h"

SandBoxLayer::SandBoxLayer()
	: Layer("SandBoxLayer"),
	m_Camera(Aurora::CreateRef<Aurora::EditorCamera>(45.0f, 16.0f / 9.0f, 0.1f, 1000.0f)),
	m_OrthoCamera(Aurora::CreateRef<Aurora::OrthoGraphicCamera>(16.0f / 9.0f, -100.0f, 100.0f))
{
}

void SandBoxLayer::onAttach()
{
	float groundVertices[] = {
		-50.0f, -5.0f, -50.0f,   0.0f, 1.0f, 0.0f,   1.0f, 1.0f, 1.0f, 1.0f,   0.0f, 0.0f,
		 50.0f, -5.0f, -50.0f,   0.0f, 1.0f, 0.0f,   1.0f, 1.0f, 1.0f, 1.0f,   1.0f, 0.0f,
		 50.0f, -5.0f,  50.0f,   0.0f, 1.0f, 0.0f,   1.0f, 1.0f, 1.0f, 1.0f,   1.0f, 1.0f,
		-50.0f, -5.0f,  50.0f,   0.0f, 1.0f, 0.0f,   1.0f, 1.0f, 1.0f, 1.0f,   0.0f, 1.0f
	};

	uint32_t groundIndices[] = { 0, 1, 2, 2, 3, 0 };

	Aurora::BufferLayout m_GroundLayout = {
		{ Aurora::ShaderDataType::Float3, "a_Position" },
		{ Aurora::ShaderDataType::Float3, "a_Normals" },
		{ Aurora::ShaderDataType::Float4, "a_Color" },
		{ Aurora::ShaderDataType::Float2, "a_TexCoord" }
	};

	// For ground...
	m_GroundVertexArray = Aurora::VertexArray::Create();
	Aurora::Ref<Aurora::VertexBuffer> m_GroundVertexBuffer = Aurora::VertexBuffer::Create(groundVertices, sizeof(groundVertices));
	m_GroundVertexBuffer->bind();
	m_GroundVertexBuffer->setLayout(m_GroundLayout);
	m_GroundVertexArray->addVertexBuffer(m_GroundVertexBuffer);
	Aurora::Ref<Aurora::IndexBuffer> m_GroundIndexBuffer = Aurora::IndexBuffer::Create(groundIndices, sizeof(groundIndices) / sizeof(uint32_t));
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

	Aurora::BufferLayout m_SphereLayout = {
		{ Aurora::ShaderDataType::Float3, "a_Pos" },
		{ Aurora::ShaderDataType::Float3, "a_Normal" },
		{ Aurora::ShaderDataType::Float2, "a_TexCoords" },
		{ Aurora::ShaderDataType::Float4, "a_Color" }
	};

	m_SphereVertexArray = Aurora::VertexArray::Create();
	Aurora::Ref<Aurora::VertexBuffer> m_SphereVertexBuffer = Aurora::VertexBuffer::Create(&data[0], (uint32_t)data.size() * sizeof(float));
	m_SphereVertexBuffer->bind();
	m_SphereVertexBuffer->setLayout(m_SphereLayout);
	m_SphereVertexArray->addVertexBuffer(m_SphereVertexBuffer);

	Aurora::Ref<Aurora::IndexBuffer> m_SphereIndexBuffer = Aurora::IndexBuffer::Create(&Sphereindices[0], (uint32_t)Sphereindices.size());
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

	m_CubePositions[0] = glm::vec3(0.0f, 0.0f, 9.0f);
	m_CubePositions[1] = glm::vec3(0.0f, 0.0f, 7.0f);
	m_CubePositions[2] = glm::vec3(0.0f, 0.0f, 5.0f);
	m_CubePositions[3] = glm::vec3(0.0f, 0.0f, 3.0f);
	m_CubePositions[4] = glm::vec3(0.0f, 0.0f, 1.0f);
	m_CubePositions[5] = glm::vec3(0.0f, 0.0f, -1.0f);
	m_CubePositions[6] = glm::vec3(0.0f, 0.0f, -3.0f);
	m_CubePositions[7] = glm::vec3(0.0f, 0.0f, -5.0f);
	m_CubePositions[8] = glm::vec3(0.0f, 0.0f, -7.0f);
	m_CubePositions[9] = glm::vec3(0.0f, 0.0f, -9.0f);

	m_Shaders.Load("resources/shaders/Basic.glsl");
	m_Shaders.Load("resources/shaders/Light.glsl");
	m_Shaders.Load("resources/shaders/Ground.glsl");
	m_Shaders.Load("resources/shaders/Sphere.glsl");

	uint32_t indices[6 * 6] = { 0, 1, 2, 2, 3, 0,
							  4, 5, 6, 6, 7, 4,
							  8, 9, 10, 10 ,11, 8,
							  12, 13, 14, 14, 15, 12,
							  16, 17, 18, 18, 19, 16,
							  20, 21, 22, 22, 23, 20 };

	Aurora::BufferLayout m_Layout = {
		{ Aurora::ShaderDataType::Float3, "a_Position" },
		{ Aurora::ShaderDataType::Float3, "a_Normals" },
		{ Aurora::ShaderDataType::Float4, "a_Color" },
		{ Aurora::ShaderDataType::Float2, "a_TexCoord" }
	};

	// For main cube
	m_VertexArray = Aurora::VertexArray::Create();

	Aurora::Ref<Aurora::VertexBuffer> m_VertexBuffer = Aurora::VertexBuffer::Create(vertices, sizeof(vertices));
	m_VertexBuffer->bind();
	m_VertexBuffer->setLayout(m_Layout);
	m_VertexArray->addVertexBuffer(m_VertexBuffer);

	Aurora::Ref<Aurora::IndexBuffer> m_IndexBuffer = Aurora::IndexBuffer::Create(indices, sizeof(indices) / sizeof(uint32_t));
	m_IndexBuffer->bind();
	m_VertexArray->setIndexBuffer(m_IndexBuffer);

	m_VertexBuffer->unBind();
	m_IndexBuffer->unBind();

	// for light source
	m_LightVertexArray = Aurora::VertexArray::Create();
	m_VertexBuffer->bind();
	m_LightVertexArray->addVertexBuffer(m_VertexBuffer);
	m_IndexBuffer->bind();
	m_LightVertexArray->setIndexBuffer(m_IndexBuffer);

	m_VertexBuffer->unBind();
	m_IndexBuffer->unBind();

	// Creating textures
	Aurora::Ref<Aurora::Texture> text1 = Aurora::Texture::Create("resources/textures/Lufi.png");
	//text1->bind(); // With OpenGL 4.5 and new CreateTextures api it is not necessary anymore to bind and unbind
	text1->flipTextureVertically(true);
	text1->setTextureWrapping(GL_REPEAT);
	text1->setTextureFiltering(GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR);
	text1->loadTextureData();
	//text1->unBind();

	Aurora::Ref<Aurora::Texture> text2 = Aurora::Texture::Create("resources/textures/Qiyana2.png");
	//text2->bind(1); // Only before the draw call we should bind the texture to its corresponding texture slot
	text2->flipTextureVertically(true);
	text2->setTextureWrapping(GL_REPEAT);
	text2->setTextureFiltering(GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR);
	text2->loadTextureData();
	//text2->unBind();

	Aurora::Ref<Aurora::Texture> text3 = Aurora::Texture::Create("resources/textures/checkerboard.png");
	//text3->bind(2);
	text3->flipTextureVertically(true);
	text3->setTextureWrapping(GL_REPEAT);
	text3->setTextureFiltering(GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR);
	text3->loadTextureData();
	//text3->unBind();

	Aurora::Ref<Aurora::Texture> text4 = Aurora::Texture::Create("resources/textures/map.jpg");
	//text4->bind(3);
	text4->flipTextureVertically(true);
	text4->setTextureWrapping(GL_REPEAT);
	text4->setTextureFiltering(GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR);
	text4->loadTextureData();
	//text4->unBind();

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

void SandBoxLayer::onDetach()
{
}

void SandBoxLayer::onUpdate(Aurora::TimeStep ts)
{
	Aurora::RenderCommand::setClearColor(m_Color);
	Aurora::RenderCommand::Clear();

	if(m_Perspective)
		Aurora::Renderer::BeginScene(m_Camera);
	else
		Aurora::Renderer::BeginScene(m_OrthoCamera);

	m_Shaders.Get("Sphere")->bind();
	m_Shaders.Get("Sphere")->setUniform4f("lightColor", m_LightColor);
	m_Shaders.Get("Sphere")->setUniform1f("blend", m_Blend);
	m_Shaders.Get("Sphere")->setUniform1f("ambientStrength", m_AmbLight);
	m_Shaders.Get("Sphere")->setUniform3f("src_pos", m_LightTranslations);
	if(m_Perspective)
		m_Shaders.Get("Sphere")->setUniform3f("view_pos", m_Camera->GetPosition());
	else
		m_Shaders.Get("Sphere")->setUniform3f("view_pos", m_OrthoCamera->GetPosition());

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

	m_Textures[3]->bind(3);
	Aurora::Renderer::DrawSphere(m_Shaders.Get("Sphere"), model, m_SphereVertexArray);

	m_Shaders.Get("Basic")->bind();
	m_Shaders.Get("Basic")->setUniform4f("lightColor", m_LightColor);
	m_Shaders.Get("Basic")->setUniform1f("blend", m_Blend);
	m_Shaders.Get("Basic")->setUniform1f("ambientStrength", m_AmbLight);
	m_Shaders.Get("Basic")->setUniform3f("src_pos", m_LightTranslations);
	if(m_Perspective)
		m_Shaders.Get("Basic")->setUniform3f("view_pos", m_Camera->GetPosition());
	else
		m_Shaders.Get("Basic")->setUniform3f("view_pos", m_OrthoCamera->GetPosition());

	m_Shaders.Get("Basic")->setUniform4f("src_color", m_LightColor);
	m_Shaders.Get("Basic")->setUniform4f("un_color", m_UniColor);
	for (uint32_t i = 0; i < m_CubePositions.size(); i++)
	{
		// calculate the model matrix for each object and pass it to shader before drawing
		model = glm::mat4(1.0f); // make sure to initialize matrix to identity matrix first
		model = glm::translate(model, m_CubePositions[i] + m_Transalations);

		if (m_IsRPressed)
			model = glm::rotate(model, (float)glfwGetTime(), glm::vec3(1.0f, 0.3f, 0.5f));
		else {
			angle = m_Rotations;
			rotation = glm::rotate(glm::mat4(1.0f), glm::radians(angle.x), { 1.0f, 0.0f, 0.0f })
				* glm::rotate(glm::mat4(1.0f), glm::radians(angle.y), { 0.0f, 1.0f, 0.0f })
				* glm::rotate(glm::mat4(1.0f), glm::radians(180.0f + angle.z), { 0.0f, 0.0f, 1.0f });
			model *= rotation;
		}

		model = glm::scale(model, m_Scales);

		m_Textures[0]->bind();
		m_Textures[1]->bind(1);
		Aurora::Renderer::DrawQuad(m_Shaders.Get("Basic"), model, m_VertexArray);
	}

	m_Shaders.Get("Ground")->bind();
	m_Shaders.Get("Ground")->setUniform4f("lightColor", m_LightColor);
	m_Shaders.Get("Ground")->setUniform1f("blend", m_Blend);
	m_Shaders.Get("Ground")->setUniform1f("ambientStrength", m_AmbLight);
	m_Shaders.Get("Ground")->setUniform3f("src_pos", m_LightTranslations);
	if (m_Perspective)
		m_Shaders.Get("Ground")->setUniform3f("view_pos", m_Camera->GetPosition());
	else
		m_Shaders.Get("Ground")->setUniform3f("view_pos", m_OrthoCamera->GetPosition());

	m_Shaders.Get("Ground")->setUniform4f("src_color", m_LightColor);
	m_Shaders.Get("Ground")->setUniform4f("un_color", m_UniColor);
	model = glm::translate(glm::mat4(1.0f), m_GroundTranslations);
	model = glm::scale(model, m_GroundScales);

	m_Textures[2]->bind(2);
	Aurora::Renderer::DrawQuad(m_Shaders.Get("Ground"), model, m_GroundVertexArray);

	m_Shaders.Get("Light")->bind();
	m_Shaders.Get("Light")->setUniform4f("lightColor", m_LightColor);
	m_Shaders.Get("Light")->setUniform1f("blend", m_Blend);
	m_Shaders.Get("Light")->setUniform1f("ambientStrength", m_AmbLight);
	m_Shaders.Get("Light")->setUniform3f("src_pos", m_LightTranslations);
	if (m_Perspective)
		m_Shaders.Get("Light")->setUniform3f("view_pos", m_Camera->GetPosition());
	else
		m_Shaders.Get("Light")->setUniform3f("view_pos", m_OrthoCamera->GetPosition());

	m_Shaders.Get("Light")->setUniform4f("src_color", m_LightColor);
	m_Shaders.Get("Light")->setUniform4f("un_color", m_UniColor);
	model = glm::translate(glm::mat4(1.0f), m_LightTranslations);
	model = glm::scale(model, m_LightScales);

	Aurora::Renderer::DrawQuad(m_Shaders.Get("Light"), model, m_LightVertexArray);

	Aurora::Renderer::EndScene();

	if (m_Perspective)
		m_Camera->OnUpdate(ts);
	else
		m_OrthoCamera->OnUpdate(ts);
}

void SandBoxLayer::onEvent(Aurora::Event& e)
{
	if (m_Perspective)
		m_Camera->OnEvent(e);
	else
		m_OrthoCamera->OnEvent(e);

	if (Aurora::Input::isKeyPressed(GLFW_KEY_R))
		m_IsRPressed = !m_IsRPressed;
}

void SandBoxLayer::onImGuiRender()
{
	Aurora::Application& app = Aurora::Application::getApp();

	ImGui::Begin("Editing Panel");
	if (ImGui::CollapsingHeader("Cube")) {
		ImGui::ColorEdit3("Uniform Color", (float*)&m_UniColor);
		ImGui::SliderFloat3("Cube Translation", (float*)&m_Transalations, 0.0f, 5.0f);
		ImGui::SliderFloat3("Cube Rotations", (float*)&m_Rotations, 0.0f, 360.0f);
		ImGui::SliderFloat3("Cube Scale", (float*)&m_Scales, 0.0f, 3.0f);
		ImGui::SliderFloat("Cube Blend", &m_Blend, 0.0f, 1.0f);
	}

	ImGui::Separator();

	if (ImGui::CollapsingHeader("Light Source")) {
		ImGui::ColorEdit3("Light Color", (float*)&m_LightColor);
		ImGui::SliderFloat3("Light Translation", (float*)&m_LightTranslations, -30.0f, 135.0f);
		ImGui::SliderFloat3("Light Scale", (float*)&m_LightScales, 0.0f, 3.0f);
	}

	ImGui::Separator();

	if (ImGui::CollapsingHeader("Ball")) {
		ImGui::SliderFloat3("Sphere Translation", (float*)&m_SphereTransalations, -50.0f, 50.0f);
		ImGui::SliderFloat3("Sphere Rotations", (float*)&m_SphereRotations, 0.0f, 360.0f);
		ImGui::SliderFloat3("Sphere Scale", (float*)&m_SphereScales, 0.0f, 30.0f);
	}

	ImGui::Separator();

	if (ImGui::CollapsingHeader("Ground")) {
		ImGui::SliderFloat3("Ground Translation", (float*)&m_GroundTranslations, -5.0f, 5.0f);
		ImGui::SliderFloat3("Ground Scale", (float*)&m_GroundScales, 0.0f, 3.0f);
	}

	ImGui::Separator();

	ImGui::ColorEdit3("Clear Color", (float*)&m_Color);
	ImGui::SliderFloat("Ambient Light", &m_AmbLight, 0.0f, 1.0f);
	ImGui::Text("Framerate: %.f", ImGui::GetIO().Framerate);

	ImGui::Separator();
	//ImGui::ShowDemoWindow(); // For reference

	ImGui::Checkbox("Camera Type:", &m_Perspective);
	ImGui::SameLine();
	if (m_Perspective)
		ImGui::Text("Perspective Camera!");
	else
		ImGui::Text("OrthoGraphic Camera!");


	ImGui::Checkbox("V Sync ", &(app.getVSync()));

	ImGui::End();
}