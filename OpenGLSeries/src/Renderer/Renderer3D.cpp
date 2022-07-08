#include "OGLpch.h"
#include "Renderer3D.h"

struct QuadVertex
{
	glm::vec3 Position;
	glm::vec4 Color;
	glm::vec2 TexCoords;
	float TextureIndex;
	float TilingFactor;
};

struct RendererData
{
	const size_t MaxQuads          = 5000;
	const size_t MaxVertices       = MaxQuads * 8;
	const size_t MaxIndices        = MaxQuads * 36;
	static const size_t MaxTextureSlots   = 32;
	//const size_t MaxTextureSlots   = RendererProperties::GetRendererProperties()->TextureSlots;

	Ref<VertexArray> QuadVertexArray;
	Ref<VertexBuffer> QuadVertexBuffer;
	Ref<Shader> TexShader;
	Ref<Texture> WhiteTex;

	uint32_t QuadIndexCount = 0;
	QuadVertex* QuadVertexBufferBase = nullptr; // This is to keep track of the base of memory allocations
	QuadVertex* QuadVertexBufferPtr  = nullptr;

	// Here the identifier will become an asset handle if i ever implement it
	std::array<Ref<Texture>, MaxTextureSlots> TextureSlots;
	uint32_t TextureSlotIndex = 1; // 0 is the white texture

	glm::vec4 QuadVertexPositions[8];
};

static RendererData s_Data;

void Renderer3D::Init()
{
	PROFILE_FUNCTION();

	s_Data.QuadVertexArray = VertexArray::Create();

	s_Data.QuadVertexBuffer = VertexBuffer::Create((uint32_t)s_Data.MaxVertices * sizeof(QuadVertex));
	s_Data.QuadVertexBuffer->setLayout({
		{ ShaderDataType::Float3, "a_Position"     },
		{ ShaderDataType::Float4, "a_Color"        },
		{ ShaderDataType::Float2, "a_TexCoord"     },
		{ ShaderDataType::Float,  "a_TexIndex"     },
		{ ShaderDataType::Float,  "a_TilingFactor" }
	});
	s_Data.QuadVertexArray->addVertexBuffer(s_Data.QuadVertexBuffer);

	s_Data.QuadVertexBufferBase = new QuadVertex[s_Data.MaxVertices];

	uint32_t* quadIndices = new uint32_t[s_Data.MaxIndices];

	uint32_t offset = 0;
	for (uint32_t i = 0; i < s_Data.MaxIndices; i += 36)
	{
		quadIndices[i +  0] = offset + 0;
		quadIndices[i +  1] = offset + 1;
		quadIndices[i +  2] = offset + 2;
		quadIndices[i +  3] = offset + 2;
		quadIndices[i +  4] = offset + 3;
		quadIndices[i +  5] = offset + 0;
						 
		quadIndices[i +  6] = offset + 4;
		quadIndices[i +  7] = offset + 5;
		quadIndices[i +  8] = offset + 6;
		quadIndices[i +  9] = offset + 6;
		quadIndices[i + 10] = offset + 7;
		quadIndices[i + 11] = offset + 4;

		quadIndices[i + 12] = offset + 7;
		quadIndices[i + 13] = offset + 6;
		quadIndices[i + 14] = offset + 2;
		quadIndices[i + 15] = offset + 2;
		quadIndices[i + 16] = offset + 3;
		quadIndices[i + 17] = offset + 7;

		quadIndices[i + 18] = offset + 4;
		quadIndices[i + 19] = offset + 5;
		quadIndices[i + 20] = offset + 1;
		quadIndices[i + 21] = offset + 1;
		quadIndices[i + 22] = offset + 0;
		quadIndices[i + 23] = offset + 4;

		quadIndices[i + 24] = offset + 7;
		quadIndices[i + 25] = offset + 3;
		quadIndices[i + 26] = offset + 0;
		quadIndices[i + 27] = offset + 0;
		quadIndices[i + 28] = offset + 4;
		quadIndices[i + 29] = offset + 7;

		quadIndices[i + 30] = offset + 5;
		quadIndices[i + 31] = offset + 1;
		quadIndices[i + 32] = offset + 2;
		quadIndices[i + 33] = offset + 2;
		quadIndices[i + 34] = offset + 6;
		quadIndices[i + 35] = offset + 5;

		offset += 8;
	}

	Ref<IndexBuffer> quadIB = IndexBuffer::Create(quadIndices, (uint32_t)s_Data.MaxIndices);
	s_Data.QuadVertexArray->setIndexBuffer(quadIB);
	delete[] quadIndices;


	s_Data.WhiteTex = Texture::Create(1, 1);
	uint32_t whiteTextureData = 0xffffffff;
	s_Data.WhiteTex->setData(&whiteTextureData, sizeof(uint32_t));

	int samplers[s_Data.MaxTextureSlots];
	for (int i = 0; i < s_Data.MaxTextureSlots; i++)
		samplers[i] = i;
	// This is the sampler that will be submitted to OpenGL and in which OpenGL will be sampling the textures from according to the passed index

	s_Data.TexShader = Shader::Create("resources/shaders/MainShader.glsl");
	s_Data.TexShader->bind();
	s_Data.TexShader->setUniformArrayi("u_Textures", samplers, s_Data.MaxTextureSlots);

	s_Data.TextureSlots[0] = s_Data.WhiteTex; // index 0 is for the white texture.

	s_Data.QuadVertexPositions[0] = { -0.5f, -0.5f, -0.5f, 1.0f };
	s_Data.QuadVertexPositions[1] = {  0.5f, -0.5f, -0.5f, 1.0f };
	s_Data.QuadVertexPositions[2] = {  0.5f,  0.5f, -0.5f, 1.0f };
	s_Data.QuadVertexPositions[3] = { -0.5f,  0.5f, -0.5f, 1.0f };
	s_Data.QuadVertexPositions[4] = { -0.5f, -0.5f,  0.5f, 1.0f };
	s_Data.QuadVertexPositions[5] = {  0.5f, -0.5f,  0.5f, 1.0f };
	s_Data.QuadVertexPositions[6] = {  0.5f,  0.5f,  0.5f, 1.0f };
	s_Data.QuadVertexPositions[7] = { -0.5f,  0.5f,  0.5f, 1.0f };
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

	s_Data.TextureSlotIndex = 1;
}

void Renderer3D::BeginScene(const Ref<OrthoGraphicCamera>& camera)
{
	PROFILE_FUNCTION();

	s_Data.TexShader->bind();
	s_Data.TexShader->setUniformMat4("u_ViewProjmatrix", camera->GetViewProjection());

	s_Data.QuadIndexCount = 0;
	s_Data.QuadVertexBufferPtr = s_Data.QuadVertexBufferBase;
	
	s_Data.TextureSlotIndex = 1;
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
	// Bind Textures
	for (uint32_t i = 0; i < s_Data.TextureSlotIndex; i++)
		s_Data.TextureSlots[i]->bind(i);

	RenderCommand::DrawIndexed(s_Data.QuadVertexArray, s_Data.QuadIndexCount);
}

void Renderer3D::DrawQuad(const glm::vec3& position, const glm::vec3& scale, const glm::vec4& color)
{
	PROFILE_FUNCTION();

	const float whiteTexIndex = 0.0f; // White texture.
	const float TilingFactor  = 1.0f; // TilingFactor.

	glm::mat4 transform = glm::translate(glm::mat4(1.0f), position)
						* glm::scale(glm::mat4(1.0f), scale);

	s_Data.QuadVertexBufferPtr->Position      = transform * s_Data.QuadVertexPositions[0];
	s_Data.QuadVertexBufferPtr->Color         = color;
	s_Data.QuadVertexBufferPtr->TexCoords     = { 0.0f, 0.0f };
	s_Data.QuadVertexBufferPtr->TextureIndex = whiteTexIndex;
	s_Data.QuadVertexBufferPtr->TilingFactor  = TilingFactor;;
	s_Data.QuadVertexBufferPtr++;			  
											  
	s_Data.QuadVertexBufferPtr->Position      = transform * s_Data.QuadVertexPositions[1];
	s_Data.QuadVertexBufferPtr->Color         = color;
	s_Data.QuadVertexBufferPtr->TexCoords     = { 1.0f, 0.0f };
	s_Data.QuadVertexBufferPtr->TextureIndex = whiteTexIndex;
	s_Data.QuadVertexBufferPtr->TilingFactor  = TilingFactor;
	s_Data.QuadVertexBufferPtr++;
											  
	s_Data.QuadVertexBufferPtr->Position      = transform * s_Data.QuadVertexPositions[2];
	s_Data.QuadVertexBufferPtr->Color         = color;
	s_Data.QuadVertexBufferPtr->TexCoords     = { 1.0f, 1.0f };
	s_Data.QuadVertexBufferPtr->TextureIndex = whiteTexIndex;
	s_Data.QuadVertexBufferPtr->TilingFactor  = TilingFactor;
	s_Data.QuadVertexBufferPtr++;

	s_Data.QuadVertexBufferPtr->Position      = transform * s_Data.QuadVertexPositions[3];
	s_Data.QuadVertexBufferPtr->Color         = color;
	s_Data.QuadVertexBufferPtr->TexCoords     = { 0.0f, 1.0f };
	s_Data.QuadVertexBufferPtr->TextureIndex = whiteTexIndex;
	s_Data.QuadVertexBufferPtr->TilingFactor  = TilingFactor;
	s_Data.QuadVertexBufferPtr++;

	s_Data.QuadVertexBufferPtr->Position      = transform * s_Data.QuadVertexPositions[4];
	s_Data.QuadVertexBufferPtr->Color         = color;
	s_Data.QuadVertexBufferPtr->TexCoords     = { 0.0f, 0.0f };
	s_Data.QuadVertexBufferPtr->TextureIndex = whiteTexIndex;
	s_Data.QuadVertexBufferPtr->TilingFactor  = TilingFactor;
	s_Data.QuadVertexBufferPtr++;

	s_Data.QuadVertexBufferPtr->Position      = transform * s_Data.QuadVertexPositions[5];
	s_Data.QuadVertexBufferPtr->Color         = color;
	s_Data.QuadVertexBufferPtr->TexCoords     = { 1.0f, 0.0f };
	s_Data.QuadVertexBufferPtr->TextureIndex = whiteTexIndex;
	s_Data.QuadVertexBufferPtr->TilingFactor  = TilingFactor;
	s_Data.QuadVertexBufferPtr++;

	s_Data.QuadVertexBufferPtr->Position      = transform * s_Data.QuadVertexPositions[6];
	s_Data.QuadVertexBufferPtr->Color         = color;
	s_Data.QuadVertexBufferPtr->TexCoords     = { 1.0f, 1.0f };
	s_Data.QuadVertexBufferPtr->TextureIndex = whiteTexIndex;
	s_Data.QuadVertexBufferPtr->TilingFactor  = TilingFactor;
	s_Data.QuadVertexBufferPtr++;

	s_Data.QuadVertexBufferPtr->Position      = transform * s_Data.QuadVertexPositions[7];
	s_Data.QuadVertexBufferPtr->Color         = color;
	s_Data.QuadVertexBufferPtr->TexCoords     = { 0.0f, 1.0f };
	s_Data.QuadVertexBufferPtr->TextureIndex = whiteTexIndex;
	s_Data.QuadVertexBufferPtr->TilingFactor  = TilingFactor;
	s_Data.QuadVertexBufferPtr++;

	s_Data.QuadIndexCount += 36;
}

void Renderer3D::DrawQuad(const glm::vec3& position, const glm::vec3& scale, const Ref<Texture>& texture, float tiling, const glm::vec4& tintcolor)
{
	PROFILE_FUNCTION();

	// textureIndex is the index that will be submitted in the VBO with everything and then passed on to the fragment shader so 
	// that the shader knows which index from the sampler to sample from.

	// So here we need to find the texture index of the passed index and check if it has already been used.
	// If it the case where it has been used before, the index will be already found in the array and we just return the index
	float textureIndex = 0.0f;
	for (uint32_t i = 1; i < s_Data.TextureSlotIndex; i++)
	{
		if (*s_Data.TextureSlots[i] == *texture)
		{
			textureIndex = (float)i;
			break;
		}
	}
	
	// s_Data.TextureSlotIndex is the next available index in the sampler
	// If the case happens that it has never been used before, here we just add that index to the array so that it can be used later.
	if (textureIndex == 0.0f)
	{
		textureIndex = (float)s_Data.TextureSlotIndex;
		s_Data.TextureSlots[s_Data.TextureSlotIndex] = texture;
		s_Data.TextureSlotIndex++;
	}

	glm::mat4 transform = glm::translate(glm::mat4(1.0f), position)
		* glm::scale(glm::mat4(1.0f), scale);

	s_Data.QuadVertexBufferPtr->Position      = transform * s_Data.QuadVertexPositions[0];
	s_Data.QuadVertexBufferPtr->Color         = tintcolor;
	s_Data.QuadVertexBufferPtr->TexCoords     = { 0.0f, 0.0f };
	s_Data.QuadVertexBufferPtr->TextureIndex = textureIndex;
	s_Data.QuadVertexBufferPtr->TilingFactor  = tiling;
	s_Data.QuadVertexBufferPtr++;

	s_Data.QuadVertexBufferPtr->Position      = transform * s_Data.QuadVertexPositions[1];
	s_Data.QuadVertexBufferPtr->Color         = tintcolor;
	s_Data.QuadVertexBufferPtr->TexCoords     = { 1.0f, 0.0f };
	s_Data.QuadVertexBufferPtr->TextureIndex = textureIndex;
	s_Data.QuadVertexBufferPtr->TilingFactor  = tiling;
	s_Data.QuadVertexBufferPtr++;

	s_Data.QuadVertexBufferPtr->Position      = transform * s_Data.QuadVertexPositions[2];
	s_Data.QuadVertexBufferPtr->Color         = tintcolor;
	s_Data.QuadVertexBufferPtr->TexCoords     = { 1.0f, 1.0f };
	s_Data.QuadVertexBufferPtr->TextureIndex = textureIndex;
	s_Data.QuadVertexBufferPtr->TilingFactor  = tiling;
	s_Data.QuadVertexBufferPtr++;

	s_Data.QuadVertexBufferPtr->Position      = transform * s_Data.QuadVertexPositions[3];
	s_Data.QuadVertexBufferPtr->Color         = tintcolor;
	s_Data.QuadVertexBufferPtr->TexCoords     = { 0.0f, 1.0f };
	s_Data.QuadVertexBufferPtr->TextureIndex = textureIndex;
	s_Data.QuadVertexBufferPtr->TilingFactor  = tiling;
	s_Data.QuadVertexBufferPtr++;

	s_Data.QuadVertexBufferPtr->Position      = transform * s_Data.QuadVertexPositions[4];
	s_Data.QuadVertexBufferPtr->Color         = tintcolor;
	s_Data.QuadVertexBufferPtr->TexCoords     = { 0.0f, 0.0f };
	s_Data.QuadVertexBufferPtr->TextureIndex = textureIndex;
	s_Data.QuadVertexBufferPtr->TilingFactor  = tiling;
	s_Data.QuadVertexBufferPtr++;

	s_Data.QuadVertexBufferPtr->Position      = transform * s_Data.QuadVertexPositions[5];
	s_Data.QuadVertexBufferPtr->Color         = tintcolor;
	s_Data.QuadVertexBufferPtr->TexCoords     = { 1.0f, 0.0f };
	s_Data.QuadVertexBufferPtr->TextureIndex = textureIndex;
	s_Data.QuadVertexBufferPtr->TilingFactor  = tiling;
	s_Data.QuadVertexBufferPtr++;

	s_Data.QuadVertexBufferPtr->Position      = transform * s_Data.QuadVertexPositions[6];
	s_Data.QuadVertexBufferPtr->Color         = tintcolor;
	s_Data.QuadVertexBufferPtr->TexCoords     = { 1.0f, 1.0f };
	s_Data.QuadVertexBufferPtr->TextureIndex = textureIndex;
	s_Data.QuadVertexBufferPtr->TilingFactor  = tiling;
	s_Data.QuadVertexBufferPtr++;

	s_Data.QuadVertexBufferPtr->Position      = transform * s_Data.QuadVertexPositions[7];
	s_Data.QuadVertexBufferPtr->Color         = tintcolor;
	s_Data.QuadVertexBufferPtr->TexCoords     = { 0.0f, 1.0f };
	s_Data.QuadVertexBufferPtr->TextureIndex = textureIndex;
	s_Data.QuadVertexBufferPtr->TilingFactor  = tiling;
	s_Data.QuadVertexBufferPtr++;

	s_Data.QuadIndexCount += 36;
}

void Renderer3D::DrawRotatedQuad(const glm::vec3& position, const glm::vec3 rotations, const glm::vec3& scale, const glm::vec4& color)// Should take a rotation!
{
	PROFILE_FUNCTION();

	const float whiteTexIndex = 0.0f; // White texture.
	const float TilingFactor = 1.0f; // TilingFactor.

	glm::mat4 transform = glm::translate(glm::mat4(1.0f), position)
		* glm::rotate(glm::mat4(1.0f), glm::radians(rotations.x), { 1.0f, 0.0f, 0.0f })
		* glm::rotate(glm::mat4(1.0f), glm::radians(rotations.y), { 0.0f, 1.0f, 0.0f })
		* glm::rotate(glm::mat4(1.0f), glm::radians(rotations.z), { 0.0f, 0.0f, 1.0f })
		* glm::scale(glm::mat4(1.0f), scale);


	s_Data.QuadVertexBufferPtr->Position = transform * s_Data.QuadVertexPositions[0];
	s_Data.QuadVertexBufferPtr->Color = color;
	s_Data.QuadVertexBufferPtr->TexCoords = { 0.0f, 0.0f };
	s_Data.QuadVertexBufferPtr->TextureIndex = whiteTexIndex;
	s_Data.QuadVertexBufferPtr->TilingFactor = TilingFactor;
	s_Data.QuadVertexBufferPtr++;

	s_Data.QuadVertexBufferPtr->Position = transform * s_Data.QuadVertexPositions[1];
	s_Data.QuadVertexBufferPtr->Color = color;
	s_Data.QuadVertexBufferPtr->TexCoords = { 1.0f, 0.0f };
	s_Data.QuadVertexBufferPtr->TextureIndex = whiteTexIndex;
	s_Data.QuadVertexBufferPtr->TilingFactor = TilingFactor;
	s_Data.QuadVertexBufferPtr++;

	s_Data.QuadVertexBufferPtr->Position = transform * s_Data.QuadVertexPositions[2];
	s_Data.QuadVertexBufferPtr->Color = color;
	s_Data.QuadVertexBufferPtr->TexCoords = { 1.0f, 1.0f };
	s_Data.QuadVertexBufferPtr->TextureIndex = whiteTexIndex;
	s_Data.QuadVertexBufferPtr->TilingFactor = TilingFactor;
	s_Data.QuadVertexBufferPtr++;

	s_Data.QuadVertexBufferPtr->Position = transform * s_Data.QuadVertexPositions[3];
	s_Data.QuadVertexBufferPtr->Color = color;
	s_Data.QuadVertexBufferPtr->TexCoords = { 0.0f, 1.0f };
	s_Data.QuadVertexBufferPtr->TextureIndex = whiteTexIndex;
	s_Data.QuadVertexBufferPtr->TilingFactor = TilingFactor;
	s_Data.QuadVertexBufferPtr++;

	s_Data.QuadVertexBufferPtr->Position = transform * s_Data.QuadVertexPositions[4];
	s_Data.QuadVertexBufferPtr->Color = color;
	s_Data.QuadVertexBufferPtr->TexCoords = { 0.0f, 0.0f };
	s_Data.QuadVertexBufferPtr->TextureIndex = whiteTexIndex;
	s_Data.QuadVertexBufferPtr->TilingFactor = TilingFactor;
	s_Data.QuadVertexBufferPtr++;

	s_Data.QuadVertexBufferPtr->Position = transform * s_Data.QuadVertexPositions[5];
	s_Data.QuadVertexBufferPtr->Color = color;
	s_Data.QuadVertexBufferPtr->TexCoords = { 1.0f, 0.0f };
	s_Data.QuadVertexBufferPtr->TextureIndex = whiteTexIndex;
	s_Data.QuadVertexBufferPtr->TilingFactor = TilingFactor;
	s_Data.QuadVertexBufferPtr++;

	s_Data.QuadVertexBufferPtr->Position = transform * s_Data.QuadVertexPositions[6];
	s_Data.QuadVertexBufferPtr->Color = color;
	s_Data.QuadVertexBufferPtr->TexCoords = { 1.0f, 1.0f };
	s_Data.QuadVertexBufferPtr->TextureIndex = whiteTexIndex;
	s_Data.QuadVertexBufferPtr->TilingFactor = TilingFactor;
	s_Data.QuadVertexBufferPtr++;

	s_Data.QuadVertexBufferPtr->Position = transform * s_Data.QuadVertexPositions[7];
	s_Data.QuadVertexBufferPtr->Color = color;
	s_Data.QuadVertexBufferPtr->TexCoords = { 0.0f, 1.0f };
	s_Data.QuadVertexBufferPtr->TextureIndex = whiteTexIndex;
	s_Data.QuadVertexBufferPtr->TilingFactor = TilingFactor;
	s_Data.QuadVertexBufferPtr++;

	s_Data.QuadIndexCount += 36;
}

void Renderer3D::DrawRotatedQuad(const glm::vec3& position, const glm::vec3 rotations, const glm::vec3& scale, const Ref<Texture>& texture, float tiling, const glm::vec4& tintColor)
{
	PROFILE_FUNCTION();

	float textureIndex = 0.0f;
	for (uint32_t i = 1; i < s_Data.TextureSlotIndex; i++)
	{
		if (*s_Data.TextureSlots[i] == *texture)
		{
			textureIndex = (float)i;
			break;
		}
	}

	if (textureIndex == 0.0f)
	{
		textureIndex = (float)s_Data.TextureSlotIndex;
		s_Data.TextureSlots[s_Data.TextureSlotIndex] = texture;
		s_Data.TextureSlotIndex++;
	}

	glm::mat4 transform = glm::translate(glm::mat4(1.0f), position)
						* glm::rotate(glm::mat4(1.0f), glm::radians(rotations.x), { 1.0f, 0.0f, 0.0f })
						* glm::rotate(glm::mat4(1.0f), glm::radians(rotations.y), { 0.0f, 1.0f, 0.0f })
						* glm::rotate(glm::mat4(1.0f), glm::radians(rotations.z), { 0.0f, 0.0f, 1.0f })
						* glm::scale(glm::mat4(1.0f), scale);
		

	s_Data.QuadVertexBufferPtr->Position = transform * s_Data.QuadVertexPositions[0];
	s_Data.QuadVertexBufferPtr->Color = tintColor;
	s_Data.QuadVertexBufferPtr->TexCoords = { 0.0f, 0.0f };
	s_Data.QuadVertexBufferPtr->TextureIndex = textureIndex;
	s_Data.QuadVertexBufferPtr->TilingFactor = tiling;
	s_Data.QuadVertexBufferPtr++;

	s_Data.QuadVertexBufferPtr->Position = transform * s_Data.QuadVertexPositions[1];
	s_Data.QuadVertexBufferPtr->Color = tintColor;
	s_Data.QuadVertexBufferPtr->TexCoords = { 1.0f, 0.0f };
	s_Data.QuadVertexBufferPtr->TextureIndex = textureIndex;
	s_Data.QuadVertexBufferPtr->TilingFactor = tiling;
	s_Data.QuadVertexBufferPtr++;

	s_Data.QuadVertexBufferPtr->Position = transform * s_Data.QuadVertexPositions[2];
	s_Data.QuadVertexBufferPtr->Color = tintColor;
	s_Data.QuadVertexBufferPtr->TexCoords = { 1.0f, 1.0f };
	s_Data.QuadVertexBufferPtr->TextureIndex = textureIndex;
	s_Data.QuadVertexBufferPtr->TilingFactor = tiling;
	s_Data.QuadVertexBufferPtr++;

	s_Data.QuadVertexBufferPtr->Position = transform * s_Data.QuadVertexPositions[3];
	s_Data.QuadVertexBufferPtr->Color = tintColor;
	s_Data.QuadVertexBufferPtr->TexCoords = { 0.0f, 1.0f };
	s_Data.QuadVertexBufferPtr->TextureIndex = textureIndex;
	s_Data.QuadVertexBufferPtr->TilingFactor = tiling;
	s_Data.QuadVertexBufferPtr++;

	s_Data.QuadVertexBufferPtr->Position = transform * s_Data.QuadVertexPositions[4];
	s_Data.QuadVertexBufferPtr->Color = tintColor;
	s_Data.QuadVertexBufferPtr->TexCoords = { 0.0f, 0.0f };
	s_Data.QuadVertexBufferPtr->TextureIndex = textureIndex;
	s_Data.QuadVertexBufferPtr->TilingFactor = tiling;
	s_Data.QuadVertexBufferPtr++;

	s_Data.QuadVertexBufferPtr->Position = transform * s_Data.QuadVertexPositions[5];
	s_Data.QuadVertexBufferPtr->Color = tintColor;
	s_Data.QuadVertexBufferPtr->TexCoords = { 1.0f, 0.0f };
	s_Data.QuadVertexBufferPtr->TextureIndex = textureIndex;
	s_Data.QuadVertexBufferPtr->TilingFactor = tiling;
	s_Data.QuadVertexBufferPtr++;

	s_Data.QuadVertexBufferPtr->Position = transform * s_Data.QuadVertexPositions[6];
	s_Data.QuadVertexBufferPtr->Color = tintColor;
	s_Data.QuadVertexBufferPtr->TexCoords = { 1.0f, 1.0f };
	s_Data.QuadVertexBufferPtr->TextureIndex = textureIndex;
	s_Data.QuadVertexBufferPtr->TilingFactor = tiling;
	s_Data.QuadVertexBufferPtr++;

	s_Data.QuadVertexBufferPtr->Position = transform * s_Data.QuadVertexPositions[7];
	s_Data.QuadVertexBufferPtr->Color = tintColor;
	s_Data.QuadVertexBufferPtr->TexCoords = { 0.0f, 1.0f };
	s_Data.QuadVertexBufferPtr->TextureIndex = textureIndex;
	s_Data.QuadVertexBufferPtr->TilingFactor = tiling;
	s_Data.QuadVertexBufferPtr++;

	s_Data.QuadIndexCount += 36;
}