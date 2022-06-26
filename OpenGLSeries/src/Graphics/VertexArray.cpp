#include "OGLpch.h"
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

void VertexArray::addVertexBuffer(const std::shared_ptr<VertexBuffer>& vertexBuffer)
{
	glBindVertexArray(m_ArrayId);
	vertexBuffer->bind();

	int index = 0;
	const auto& layout = vertexBuffer->getBufferLayout();
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

	m_VertexBuffers.push_back(vertexBuffer);
	glBindVertexArray(0);
}

void VertexArray::setIndexBuffer(const std::shared_ptr<IndexBuffer>& indexBuffer)
{
	glBindVertexArray(m_ArrayId);
	indexBuffer->bind();

	m_IndexBuffer = indexBuffer;
	glBindVertexArray(0);
}