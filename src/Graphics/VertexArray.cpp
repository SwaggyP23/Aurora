#include "VertexArray.h"

VertexArray::VertexArray()
	: m_IndexBuffer()
{
	glCreateVertexArrays(1, &m_ArrayId);
}

VertexArray::~VertexArray()
{
	glDeleteVertexArrays(1, &m_ArrayId);
}

void VertexArray::bind() const
{
	glBindVertexArray(m_ArrayId);
}

void VertexArray::unBind() const
{
	glBindVertexArray(0);
}

void VertexArray::addVertexBuffer(const VertexBuffer& vertexbuffer)
{
	glBindVertexArray(m_ArrayId);
	vertexbuffer.bind();

	int index = 0;
	const auto& layout = vertexbuffer.getBufferLayout();
	for (const auto& element : layout)
	{
		glVertexAttribPointer(index,
			element.getComponentCount(),
			ShaderDataTypeToOpenGLType(element.type),
			element.normalized ? GL_TRUE : GL_FALSE,
			layout.getStride(),
			(const void*)element.offset);

		glEnableVertexAttribArray(index++);
	}

	m_VertexBuffers.push_back(vertexbuffer);
	glBindVertexArray(0);
}

void VertexArray::setIndexBuffer(const IndexBuffer& indexbuffer)
{
	glBindVertexArray(m_ArrayId);
	indexbuffer.bind();

	m_IndexBuffer = indexbuffer;
	glBindVertexArray(0);
}