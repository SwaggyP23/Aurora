#include "Aurorapch.h"
#include "VertexArray.h"

namespace Aurora {

	Ref<VertexArray> VertexArray::Create()
	{
		return CreateRef<VertexArray>();
	}

	VertexArray::VertexArray()
		: m_IndexBuffer()
	{
		AR_OP_PROF_FUNCTION();

		glCreateVertexArrays(1, &m_ArrayId);
	}

	VertexArray::~VertexArray()
	{
		AR_OP_PROF_FUNCTION();

		glDeleteVertexArrays(1, &m_ArrayId);
	}

	void VertexArray::bind() const
	{
		AR_OP_PROF_FUNCTION();

		glBindVertexArray(m_ArrayId);
	}

	void VertexArray::unBind() const
	{
		glBindVertexArray(0);
	}

	void VertexArray::addVertexBuffer(const Ref<VertexBuffer>& vertexBuffer)
	{
		AR_OP_PROF_FUNCTION();

		AR_CORE_ASSERT(vertexBuffer->getBufferLayout().getElements().size(), "Vertex Buffer had no layout!");

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
	}

	void VertexArray::setIndexBuffer(const Ref<IndexBuffer>& indexBuffer)
	{
		AR_OP_PROF_FUNCTION();

		glBindVertexArray(m_ArrayId);
		indexBuffer->bind();

		m_IndexBuffer = indexBuffer;
	}

}