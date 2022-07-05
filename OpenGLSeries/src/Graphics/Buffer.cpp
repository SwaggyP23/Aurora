#include "OGLpch.h"
#include "Buffer.h"

//////////////////////////
// BUFFER ELEMENT!!
//////////////////////////

uint32_t BufferElement::getComponentCount() const
{
	switch (type)
	{
		case ShaderDataType::Float:   return 1;
		case ShaderDataType::Float2:  return 2;
		case ShaderDataType::Float3:  return 3;
		case ShaderDataType::Float4:  return 4;
		case ShaderDataType::Mat3:    return 3 * 3;
		case ShaderDataType::Mat4:    return 4 * 4;
		case ShaderDataType::Int:     return 1;
		case ShaderDataType::Int2:    return 2;
		case ShaderDataType::Int3:    return 3;
		case ShaderDataType::Int4:    return 4;
	}

	CORE_ASSERT(false, "Unkown Shader Data Type!");
	return 0;
}

//////////////////////////
// BUFFER LAYOUT!!
//////////////////////////

void BufferLayout::calcStrideAndOffset()
{
	PROFILE_FUNCTION();

	GLuint offset = 0;
	m_Stride = 0;
	for (auto& element : m_Elements)
	{
		element.offset = offset;
		offset += element.size;
		m_Stride += element.size;
	}
}

//////////////////////////
// VERTEX BUFFER!!
//////////////////////////

Ref<VertexBuffer> VertexBuffer::Create(GLfloat* vertices, size_t size)
{
	return CreateRef<VertexBuffer>(vertices, size);
}

VertexBuffer::VertexBuffer(size_t size)
{
	glCreateBuffers(1, &m_BufferID);
	glBindBuffer(GL_ARRAY_BUFFER, m_BufferID);
	glBufferData(GL_ARRAY_BUFFER, size, nullptr, GL_DYNAMIC_DRAW);
}

VertexBuffer::VertexBuffer(GLfloat* vertices, size_t size)
{
	PROFILE_FUNCTION();

	glCreateBuffers(1, &m_BufferID);
	glBindBuffer(GL_ARRAY_BUFFER, m_BufferID);
	glBufferData(GL_ARRAY_BUFFER, size, (const void*)vertices, GL_STATIC_DRAW);
}

VertexBuffer::~VertexBuffer()
{
	PROFILE_FUNCTION();

	glDeleteBuffers(1, &m_BufferID);
}

void VertexBuffer::bind() const
{
	PROFILE_FUNCTION();

	glBindBuffer(GL_ARRAY_BUFFER, m_BufferID);
}

void VertexBuffer::unBind() const
{
	PROFILE_FUNCTION();

	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void VertexBuffer::setLayout(const BufferLayout& layout)
{
	m_Layout = layout;
}

//////////////////////////
// INDEX BUFFER!!
//////////////////////////

Ref<IndexBuffer> IndexBuffer::Create(GLuint* indices, size_t count)
{
	return CreateRef<IndexBuffer>(indices, count);
}

IndexBuffer::IndexBuffer(GLuint* indices, size_t count)
	: m_Count(count)
{
	PROFILE_FUNCTION();

	glCreateBuffers(1, &m_BufferId);
	// Since we specified here that is is a GL_ELEMENT_ARRAY_BUFFER, a VAO must be bound when this is created 
	// otherwise it will not work
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_BufferId);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(GLuint), indices, GL_STATIC_DRAW);
}

IndexBuffer::~IndexBuffer()
{
	PROFILE_FUNCTION();

	glDeleteBuffers(1, &m_BufferId);
}

void IndexBuffer::bind() const
{
	PROFILE_FUNCTION();

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_BufferId);
}

void IndexBuffer::unBind() const
{
	PROFILE_FUNCTION();

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}