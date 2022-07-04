#include "BatchRenderer.h"

BatchRenderer::BatchRenderer()
	: Layer("BatchRenderer"),
	m_Camera(CreateRef<EditorCamera>(45.0f, 16.0f / 9.0f, 0.1f, 1000.0f)),
	m_OrthoCamera(CreateRef<OrthoGraphicCamera>(16.0f / 9.0f, -100.0f, 100.0f))
{
}

void BatchRenderer::onAttach()
{
	float vertices[] = {
		-0.5f, -0.5f, -0.5f,  1.0f, 1.0f, 1.0f, 1.0f,
		 0.5f, -0.5f, -0.5f,  1.0f, 1.0f, 1.0f, 1.0f,
		 0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 1.0f, 1.0f,
		-0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 1.0f, 1.0f,
							 
		-0.5f, -0.5f,  0.5f,  1.0f, 1.0f, 1.0f, 1.0f,
		 0.5f, -0.5f,  0.5f,  1.0f, 1.0f, 1.0f, 1.0f,
		 0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 1.0f, 1.0f,
		-0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 1.0f, 1.0f,
							 
		-0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 1.0f, 1.0f,
		-0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 1.0f, 1.0f,
		-0.5f, -0.5f, -0.5f,  1.0f, 1.0f, 1.0f, 1.0f,
		-0.5f, -0.5f,  0.5f,  1.0f, 1.0f, 1.0f, 1.0f,
							 
		 0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 1.0f, 1.0f,
		 0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 1.0f, 1.0f,
		 0.5f, -0.5f, -0.5f,  1.0f, 1.0f, 1.0f, 1.0f,
		 0.5f, -0.5f,  0.5f,  1.0f, 1.0f, 1.0f, 1.0f,
							 
		-0.5f, -0.5f, -0.5f,  1.0f, 1.0f, 1.0f, 1.0f,
		 0.5f, -0.5f, -0.5f,  1.0f, 1.0f, 1.0f, 1.0f,
		 0.5f, -0.5f,  0.5f,  1.0f, 1.0f, 1.0f, 1.0f,
		-0.5f, -0.5f,  0.5f,  1.0f, 1.0f, 1.0f, 1.0f,
							 
		-0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 1.0f, 1.0f,
		 0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 1.0f, 1.0f,
		 0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 1.0f, 1.0f,
		-0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 1.0f, 1.0f
	};

	GLuint indices[6 * 6] = { 0, 1, 2, 2, 3, 0,
							  4, 5, 6, 6, 7, 4,
							  8, 9, 10, 10 ,11, 8,
							  12, 13, 14, 14, 15, 12,
							  16, 17, 18, 18, 19, 16,
							  20, 21, 22, 22, 23, 20 };

	BufferLayout m_Layout = {
		{ ShaderDataType::Float3, "a_Position" },
		{ ShaderDataType::Float4, "a_Color" },
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

	// Creating textures
	m_Texture = Texture::Create("resources/textures/NewYork.png");
	m_Texture->bind(/*0*/);
	m_Texture->flipTextureVertically(true);
	m_Texture->setTextureWrapping(GL_REPEAT);
	m_Texture->setTextureFiltering(GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR);
	m_Texture->loadTextureData();
	m_Texture->unBind();

	m_Shader = CreateRef<Shader>("resources/shaders/MainShader.shader");
	m_Shader->bind();
	m_Shader->setUniform1i("texture1", 0);
	m_Shader->setUniform1i("texture2", 1);
	m_Shader->unBind();

}
void BatchRenderer::onDetach()
{
}

void BatchRenderer::onUpdate(TimeStep ts)
{
	RenderCommand::setClearColor(m_Color);
	RenderCommand::Clear();

	if (m_Perspective)
		Renderer::BeginScene(m_Camera);
	else
		Renderer::BeginScene(m_OrthoCamera);

	m_Shader->bind();
	m_Shader->setUniform4f("u_CubeColor", m_UniColor);

	glm::mat4 model(1.0f);
	model = glm::translate(glm::mat4(1.0f), m_Transalations);
	glm::vec3 angle = m_Rotations;
	glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), glm::radians(angle.x), { 1.0f, 0.0f, 0.0f })
		* glm::rotate(glm::mat4(1.0f), glm::radians(angle.y), { 0.0f, 1.0f, 0.0f })
		* glm::rotate(glm::mat4(1.0f), glm::radians(180.0f + angle.z), { 0.0f, 0.0f, 1.0f });

	model *= rotation;
	model = glm::scale(model, m_Scales);

	Renderer::DrawQuad(m_Shader, model, m_VertexArray);

	Renderer::EndScene();

	if (m_Perspective)
		m_Camera->OnUpdate(ts);
	else
		m_OrthoCamera->OnUpdate(ts);
}

void BatchRenderer::onEvent(Event& e)
{
	if (m_Perspective)
		m_Camera->OnEvent(e);
	else
		m_OrthoCamera->OnEvent(e);
}

void BatchRenderer::onImGuiRender()
{
	Application& app = Application::getApp();

	ImGui::Begin("Editing Panel");
	if (ImGui::CollapsingHeader("Cube")) {
		ImGui::ColorEdit3("Uniform Color", (float*)&m_UniColor);
		ImGui::SliderFloat3("Cube Translation", (float*)&m_Transalations, 0.0f, 5.0f);
		ImGui::SliderFloat3("Cube Rotations", (float*)&m_Rotations, 0.0f, 360.0f);
		ImGui::SliderFloat3("Cube Scale", (float*)&m_Scales, 0.0f, 3.0f);
	}

	ImGui::Separator();

	ImGui::ColorEdit3("Clear Color", (float*)&m_Color);
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