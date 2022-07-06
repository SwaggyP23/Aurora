#include "OGLpch.h"
#include "Renderer3D.h"

// For now just to make the batch renederer work, we wont take a scale, the scale will actually be a vec2 size which will be 
// added to the transform components!

static const size_t MaxQuadCount     = 1000;
static const size_t MaxVertexCount   = MaxQuadCount * 4;
static const size_t MaxIndexCount    = MaxQuadCount * 6;
static const size_t MaxTextureSlots  = RendererProperties::GetRendererProperties()->TextureSlots;

struct Vertex
{
	glm::vec3 Position;
	glm::vec4 Color;
	glm::vec2 TexCoords;
	float TexIndex;
};

struct QuadData
{
	Ref<VertexArray> quadVA;
	Ref<Shader> TexShader;
	Ref<Texture> DefaultTex;
};

static QuadData* s_QuadData;

void Renderer3D::Init()
{
	PROFILE_FUNCTION();

	s_QuadData = new QuadData();

	// For Cubes-----------------------------------------------------------------------
	s_QuadData->TexShader = Shader::Create("resources/shaders/MainShader.shader");
	s_QuadData->quadVA = VertexArray::Create();

	float vertices[] = {
		-0.5f, -0.5f, -0.5f,   0.0f, 0.0f,
		 0.5f, -0.5f, -0.5f,   1.0f, 0.0f,
		 0.5f,  0.5f, -0.5f,   1.0f, 1.0f,
		-0.5f,  0.5f, -0.5f,   0.0f, 1.0f,

		-0.5f, -0.5f,  0.5f,   0.0f, 0.0f,
		 0.5f, -0.5f,  0.5f,   1.0f, 0.0f,
		 0.5f,  0.5f,  0.5f,   1.0f, 1.0f,
		-0.5f,  0.5f,  0.5f,   0.0f, 1.0f,

		-0.5f,  0.5f,  0.5f,   1.0f, 0.0f,
		-0.5f,  0.5f, -0.5f,   1.0f, 1.0f,
		-0.5f, -0.5f, -0.5f,   0.0f, 1.0f,
		-0.5f, -0.5f,  0.5f,   0.0f, 0.0f,

		 0.5f,  0.5f,  0.5f,   1.0f, 0.0f,
		 0.5f,  0.5f, -0.5f,   1.0f, 1.0f,
		 0.5f, -0.5f, -0.5f,   0.0f, 1.0f,
		 0.5f, -0.5f,  0.5f,   0.0f, 0.0f,

		-0.5f, -0.5f, -0.5f,   0.0f, 1.0f,
		 0.5f, -0.5f, -0.5f,   1.0f, 1.0f,
		 0.5f, -0.5f,  0.5f,   1.0f, 0.0f,
		-0.5f, -0.5f,  0.5f,   0.0f, 0.0f,

		-0.5f,  0.5f, -0.5f,   0.0f, 1.0f,
		 0.5f,  0.5f, -0.5f,   1.0f, 1.0f,
		 0.5f,  0.5f,  0.5f,   1.0f, 0.0f,
		-0.5f,  0.5f,  0.5f,   0.0f, 0.0f
	};

	GLuint indices[6 * 6] = { 0, 1, 2, 2, 3, 0,
							  4, 5, 6, 6, 7, 4,
							  8, 9, 10, 10 ,11, 8,
							  12, 13, 14, 14, 15, 12,
							  16, 17, 18, 18, 19, 16,
							  20, 21, 22, 22, 23, 20 };

	BufferLayout m_Layout = {
		{ ShaderDataType::Float3, "a_Position" },
		{ ShaderDataType::Float2, "a_TexCoords" }
	};

	// For main cube

	Ref<VertexBuffer> vertexBuffer = VertexBuffer::Create(vertices, sizeof(vertices));
	vertexBuffer->bind();
	vertexBuffer->setLayout(m_Layout);
	s_QuadData->quadVA->addVertexBuffer(vertexBuffer);

	Ref<IndexBuffer> m_IndexBuffer = IndexBuffer::Create(indices, sizeof(indices) / sizeof(GLuint));
	m_IndexBuffer->bind();
	s_QuadData->quadVA->setIndexBuffer(m_IndexBuffer);

	vertexBuffer->unBind();
	m_IndexBuffer->unBind();

	s_QuadData->DefaultTex = CreateRef<Texture>(1, 1);
	uint32_t data = 0xffffffff;
	s_QuadData->DefaultTex->setData(&data);

	s_QuadData->TexShader->bind();
	s_QuadData->TexShader->setUniform1i("u_Texture", 0);
	s_QuadData->TexShader->unBind();
}

void Renderer3D::ShutDown()
{
	delete s_QuadData;
}

void Renderer3D::BeginScene(const Ref<EditorCamera>& camera)
{
	PROFILE_FUNCTION();

	s_QuadData->TexShader->bind();
	s_QuadData->TexShader->setUniformMat4("u_ViewProjmatrix", camera->GetViewProjection());
}

void Renderer3D::BeginScene(const Ref<OrthoGraphicCamera>& camera)
{
	PROFILE_FUNCTION();

	s_QuadData->TexShader->bind();
	s_QuadData->TexShader->setUniformMat4("u_ViewProjmatrix", camera->GetViewProjection());
}

void Renderer3D::EndScene()
{
}

void Renderer3D::DrawQuad(const glm::vec3& position, const glm::vec3& scale, const glm::vec4& color)
{
	PROFILE_FUNCTION();

	glm::mat4 model(1.0f);
	model = glm::translate(glm::mat4(1.0f), position)
		  * glm::scale(glm::mat4(1.0f), scale);

	s_QuadData->TexShader->setUniform4f("u_Color", color);
	s_QuadData->TexShader->setUniformMat4("u_ModelMatrix", model);

	s_QuadData->DefaultTex->bind();
	s_QuadData->quadVA->bind();
	RenderCommand::DrawIndexed(s_QuadData->quadVA);
}

void Renderer3D::DrawQuad(const glm::vec3& position, const glm::vec3& scale, const Ref<Texture>& texture, float tiling)
{
	PROFILE_FUNCTION();

	glm::mat4 model(1.0f);
	model = glm::translate(glm::mat4(1.0f), position)
		  * glm::scale(glm::mat4(1.0f), scale);

	s_QuadData->TexShader->setUniform4f("u_Color", glm::vec4(1.0f));
	s_QuadData->TexShader->setUniform1f("u_TilingFactor", tiling);
	s_QuadData->TexShader->setUniformMat4("u_ModelMatrix", model);

	texture->bind();
	s_QuadData->quadVA->bind();
	RenderCommand::DrawIndexed(s_QuadData->quadVA);
}

void Renderer3D::DrawRotatedQuad(const glm::vec3& position, const glm::vec3 rotations, const glm::vec3& scale, const glm::vec4& color)// Should take a rotation!
{
	PROFILE_FUNCTION();

	glm::mat4 model(1.0f);
	model = glm::translate(glm::mat4(1.0f), position) 
		  * glm::rotate(glm::mat4(1.0f), rotations.x, { 1.0f, 0.0f, 0.0f })
		  * glm::rotate(glm::mat4(1.0f), rotations.y, { 0.0f, 1.0f, 0.0f })
		  * glm::rotate(glm::mat4(1.0f), rotations.z, { 0.0f, 0.0f, 1.0f })
		  * glm::scale(glm::mat4(1.0f), scale);

	s_QuadData->TexShader->setUniform4f("u_Color", color);
	s_QuadData->TexShader->setUniformMat4("u_ModelMatrix", model);

	s_QuadData->DefaultTex->bind();
	s_QuadData->quadVA->bind();
	RenderCommand::DrawIndexed(s_QuadData->quadVA);
}

void Renderer3D::DrawRotatedQuad(const glm::vec3& position, const glm::vec3 rotations, const glm::vec3& scale, const Ref<Texture>& texture, float tiling)
{
	PROFILE_FUNCTION();

	glm::mat4 model(1.0f);
	model = glm::translate(glm::mat4(1.0f), position)
		* glm::rotate(glm::mat4(1.0f), rotations.x, { 1.0f, 0.0f, 0.0f })
		* glm::rotate(glm::mat4(1.0f), rotations.y, { 0.0f, 1.0f, 0.0f })
		* glm::rotate(glm::mat4(1.0f), rotations.z, { 0.0f, 0.0f, 1.0f })
		* glm::scale(glm::mat4(1.0f), scale);

	s_QuadData->TexShader->setUniform4f("u_Color", glm::vec4(1.0f));
	s_QuadData->TexShader->setUniform1f("u_TilingFactor", tiling);
	s_QuadData->TexShader->setUniformMat4("u_ModelMatrix", model);

	texture->bind();
	s_QuadData->quadVA->bind();
	RenderCommand::DrawIndexed(s_QuadData->quadVA);
}