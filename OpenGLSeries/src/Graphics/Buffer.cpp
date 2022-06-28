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

	CORE_LOG_ERROR("Unkown Shader Data Type!! {0}", __FUNCTION__);
	return 0;
}

//////////////////////////
// BUFFER LAYOUT!!
//////////////////////////

void BufferLayout::calcStrideAndOffset()
{
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

VertexBuffer::VertexBuffer(GLfloat* vertices, size_t size)
{
	glGenBuffers(1, &m_BufferID);
	glBindBuffer(GL_ARRAY_BUFFER, m_BufferID);
	glBufferData(GL_ARRAY_BUFFER, size, (const void*)vertices, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

VertexBuffer::~VertexBuffer()
{
	glDeleteBuffers(1, &m_BufferID);
}

void VertexBuffer::bind() const
{
	glBindBuffer(GL_ARRAY_BUFFER, m_BufferID);
}

void VertexBuffer::unBind() const
{
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void VertexBuffer::setLayout(const BufferLayout& layout)
{
	m_Layout = layout;
}

//////////////////////////
// INDEX BUFFER!!
//////////////////////////

IndexBuffer::IndexBuffer(GLuint* indices, size_t count)
	: m_Count(count)
{
	glGenBuffers(1, &m_BufferId);
	// Since we specified here that is is a GL_ELEMENT_ARRAY_BUFFER, a VAO must be bound when this is created 
	// otherwise it will not work
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_BufferId);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(GLuint), indices, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

IndexBuffer::~IndexBuffer()
{
	glDeleteBuffers(1, &m_BufferId);
}

void IndexBuffer::bind() const
{
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_BufferId);
}

void IndexBuffer::unBind() const
{
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}