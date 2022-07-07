#include "OGLpch.h"
#include "Renderer3D.h"

//float vertices[] = {
//	-0.5f, -0.5f, -0.5f,   0.0f, 0.0f,
//	 0.5f, -0.5f, -0.5f,   1.0f, 0.0f,
//	 0.5f,  0.5f, -0.5f,   1.0f, 1.0f,
//	-0.5f,  0.5f, -0.5f,   0.0f, 1.0f,
//
//	-0.5f, -0.5f,  0.5f,   0.0f, 0.0f,
//	 0.5f, -0.5f,  0.5f,   1.0f, 0.0f,
//	 0.5f,  0.5f,  0.5f,   1.0f, 1.0f,
//	-0.5f,  0.5f,  0.5f,   0.0f, 1.0f,
//
//	-0.5f,  0.5f,  0.5f,   1.0f, 0.0f,
//	-0.5f,  0.5f, -0.5f,   1.0f, 1.0f,
//	-0.5f, -0.5f, -0.5f,   0.0f, 1.0f,
//	-0.5f, -0.5f,  0.5f,   0.0f, 0.0f,
//
//	 0.5f,  0.5f,  0.5f,   1.0f, 0.0f,
//	 0.5f,  0.5f, -0.5f,   1.0f, 1.0f,
//	 0.5f, -0.5f, -0.5f,   0.0f, 1.0f,
//	 0.5f, -0.5f,  0.5f,   0.0f, 0.0f,
//
//	-0.5f, -0.5f, -0.5f,   0.0f, 1.0f,
//	 0.5f, -0.5f, -0.5f,   1.0f, 1.0f,
//	 0.5f, -0.5f,  0.5f,   1.0f, 0.0f,
//	-0.5f, -0.5f,  0.5f,   0.0f, 0.0f,
//
//	-0.5f,  0.5f, -0.5f,   0.0f, 1.0f,
//	 0.5f,  0.5f, -0.5f,   1.0f, 1.0f,
//	 0.5f,  0.5f,  0.5f,   1.0f, 0.0f,
//	-0.5f,  0.5f,  0.5f,   0.0f, 0.0f
//};
//
//
//float CubeVertices[] = {
//	-0.5f, -0.5f, -0.5f, // back bottom left
//	 0.5f, -0.5f, -0.5f, // back bottom right
//	 0.5f,  0.5f, -0.5f, // back top right
//	-0.5f,  0.5f, -0.5f, // back top left
//
//	-0.5f, -0.5f,  0.5f, // front bottom left
//	 0.5f, -0.5f,  0.5f, // front bottom right
//	 0.5f,  0.5f,  0.5f, // front top right
//	-0.5f,  0.5f,  0.5f  // front top left
//};
//
//GLuint CubeIndices[] = {
//	0, 1, 2, 2, 3, 0, // The square on the behind
//	4, 5, 6, 6, 7, 4, // The square on the front
//	7, 6, 2, 2, 3, 7, // The square on the top
//	4, 5, 1, 1, 0, 4, // The square on the bottom
//	7, 3, 0, 0, 4, 7, // The square on the left
//	5, 1, 2, 2, 6, 5  // The square on the right
//};
//
//GLuint indices[6 * 6] = { 0, 1, 2, 2, 3, 0,
//						  4, 5, 6, 6, 7, 4,
//						  8, 9, 10, 10 ,11, 8,
//						  12, 13, 14, 14, 15, 12,
//						  16, 17, 18, 18, 19, 16,
//						  20, 21, 22, 22, 23, 20 };

// For now just to make the batch renederer work, we wont take a scale, the scale will actually be a vec2 size which will be 
// added to the transform components!

struct QuadVertex
{
	glm::vec3 Position;
	glm::vec4 Color;
	glm::vec2 TexCoords;
	// float TexIndex;
};

struct RendererData
{
	const size_t MaxQuads          = 10000;
	const size_t MaxVertices       = MaxQuads * 4;
	const size_t MaxIndices        = MaxQuads * 6;
	//const size_t MaxVertexCount    = MaxQuads * 8;
	//const size_t MaxIndexCount     = MaxQuads * 36;
	const size_t MaxTextureSlots   = 32;
	//const size_t MaxTextureSlots   = RendererProperties::GetRendererProperties()->TextureSlots;

	Ref<VertexArray> QuadVertexArray;
	Ref<VertexBuffer> QuadVertexBuffer;
	Ref<Shader> TexShader;
	Ref<Texture> WhiteTex;

	uint32_t QuadIndexCount = 0;
	QuadVertex* QuadVertexBufferBase = nullptr; // This is to keep track of the base of memory allocations
	QuadVertex* QuadVertexBufferPtr  = nullptr;
};

static RendererData s_Data;

void Renderer3D::Init()
{
	PROFILE_FUNCTION();

	s_Data.QuadVertexArray = VertexArray::Create();

	s_Data.QuadVertexBuffer = VertexBuffer::Create((uint32_t)s_Data.MaxVertices * sizeof(QuadVertex));
	s_Data.QuadVertexBuffer->setLayout({
		{ ShaderDataType::Float3, "a_Position" },
		{ ShaderDataType::Float4, "a_Color" },
		{ ShaderDataType::Float2, "a_TexCoord" }
	});
	s_Data.QuadVertexArray->addVertexBuffer(s_Data.QuadVertexBuffer);

	s_Data.QuadVertexBufferBase = new QuadVertex[s_Data.MaxVertices];

	uint32_t* quadIndices = new uint32_t[s_Data.MaxIndices];

	uint32_t offset = 0;
	for (uint32_t i = 0; i < s_Data.MaxIndices; i += 6)
	{
		quadIndices[i + 0] = offset + 0;
		quadIndices[i + 1] = offset + 1;
		quadIndices[i + 2] = offset + 2;

		quadIndices[i + 3] = offset + 2;
		quadIndices[i + 4] = offset + 3;
		quadIndices[i + 5] = offset + 0;

		offset += 4;
	}

	Ref<IndexBuffer> quadIB = IndexBuffer::Create(quadIndices, (uint32_t)s_Data.MaxIndices);
	s_Data.QuadVertexArray->setIndexBuffer(quadIB);
	delete[] quadIndices;


	s_Data.WhiteTex = Texture::Create(1, 1);
	uint32_t whiteTextureData = 0xffffffff;
	s_Data.WhiteTex->setData(&whiteTextureData, sizeof(uint32_t));

	s_Data.TexShader = Shader::Create("resources/shaders/MainShader.shader");
	s_Data.TexShader->bind();
	s_Data.TexShader->setUniform1i("u_Texture", 0);
}

void Renderer3D::ShutDown()
{
}

void Renderer3D::BeginScene(const Ref<EditorCamera>& camera)
{
	PROFILE_FUNCTION();

	s_Data.TexShader->bind();
	s_Data.TexShader->setUniformMat4("u_ViewProjmatrix", camera->GetViewProjection());

	s_Data.QuadIndexCount = 0;
	s_Data.QuadVertexBufferPtr = s_Data.QuadVertexBufferBase;
}

void Renderer3D::BeginScene(const Ref<OrthoGraphicCamera>& camera)
{
	PROFILE_FUNCTION();

	s_Data.TexShader->bind();
	s_Data.TexShader->setUniformMat4("u_ViewProjmatrix", camera->GetViewProjection());

	s_Data.QuadIndexCount = 0;
	s_Data.QuadVertexBufferPtr = s_Data.QuadVertexBufferBase;
}

void Renderer3D::EndScene()
{
	PROFILE_FUNCTION();

	// Casting to one byte to actually do a correct calculation, otherwise it tells you the number of elements only since these are QuadVertex.
	uint32_t dataSize = (uint32_t)((uint8_t*)s_Data.QuadVertexBufferPtr - (uint8_t*)s_Data.QuadVertexBufferBase);
	s_Data.QuadVertexBuffer->SetData(s_Data.QuadVertexBufferBase, dataSize);

	// TODO: Handle the case if we exceeded the texture slots or the amount of quads
	Flush();
}

void Renderer3D::Flush()
{
	RenderCommand::DrawIndexed(s_Data.QuadVertexArray, s_Data.QuadIndexCount);
}

void Renderer3D::DrawQuad(const glm::vec3& position, const glm::vec3& size, const glm::vec4& color)
{
	PROFILE_FUNCTION();

	s_Data.QuadVertexBufferPtr->Position  = position;
	s_Data.QuadVertexBufferPtr->Color     = color;
	s_Data.QuadVertexBufferPtr->TexCoords = { 0.0f, 0.0f };
	s_Data.QuadVertexBufferPtr++;

	s_Data.QuadVertexBufferPtr->Position = { position.x + size.x, position.y, 0.0f };
	s_Data.QuadVertexBufferPtr->Color = color;
	s_Data.QuadVertexBufferPtr->TexCoords = { 1.0f, 0.0f };
	s_Data.QuadVertexBufferPtr++;

	s_Data.QuadVertexBufferPtr->Position = { position.x + size.x, position.y + size.y, 0.0f };
	s_Data.QuadVertexBufferPtr->Color = color;
	s_Data.QuadVertexBufferPtr->TexCoords = { 1.0f, 1.0f };
	s_Data.QuadVertexBufferPtr++;

	s_Data.QuadVertexBufferPtr->Position = { position.x, position.y + size.y, 0.0f };
	s_Data.QuadVertexBufferPtr->Color = color;
	s_Data.QuadVertexBufferPtr->TexCoords = { 0.0f, 1.0f };
	s_Data.QuadVertexBufferPtr++;

	s_Data.QuadIndexCount += 6;

	/*s_Data.TexShader->setUniform1f("u_TilingFacotr", 1.0f);

	glm::mat4 model(1.0f);
	model = glm::translate(glm::mat4(1.0f), position)
		* glm::scale(glm::mat4(1.0f), { size.x, size.y, size.z });

	s_Data.TexShader->setUniformMat4("u_ModelMatrix", model);

	s_Data.WhiteTex->bind();
	s_Data.QuadVertexArray->bind();
	RenderCommand::DrawIndexed(s_Data.QuadVertexArray);*/
}

void Renderer3D::DrawQuad(const glm::vec3& position, const glm::vec3& size, const Ref<Texture>& texture, float tiling)
{
	PROFILE_FUNCTION();

	glm::mat4 model(1.0f);
	model = glm::translate(glm::mat4(1.0f), position)
		  * glm::scale(glm::mat4(1.0f), { size.x, size.y, size.z });

	s_Data.TexShader->setUniform4f("u_Color", glm::vec4(1.0f));
	s_Data.TexShader->setUniform1f("u_TilingFactor", tiling);
	s_Data.TexShader->setUniformMat4("u_ModelMatrix", model);

	texture->bind();
	s_Data.QuadVertexArray->bind();
	RenderCommand::DrawIndexed(s_Data.QuadVertexArray);
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

	s_Data.TexShader->setUniform4f("u_Color", color);
	s_Data.TexShader->setUniformMat4("u_ModelMatrix", model);

	s_Data.WhiteTex->bind();
	s_Data.QuadVertexArray->bind();
	RenderCommand::DrawIndexed(s_Data.QuadVertexArray);
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

	s_Data.TexShader->setUniform4f("u_Color", glm::vec4(1.0f));
	s_Data.TexShader->setUniform1f("u_TilingFactor", tiling);
	s_Data.TexShader->setUniformMat4("u_ModelMatrix", model);

	texture->bind();
	s_Data.QuadVertexArray->bind();
	RenderCommand::DrawIndexed(s_Data.QuadVertexArray);
}