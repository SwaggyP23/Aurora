#include "Aurorapch.h"
#include "VertexArray.h"

namespace Aurora {

	Ref<VertexArray> VertexArray::Create()
	{
		return CreateRef<VertexArray>(1);
	}

	VertexArray::VertexArray(uint32_t count)
		: m_IndexBuffer()
	{
		AR_PROFILE_FUNCTION();

		glCreateVertexArrays(count, &m_ArrayId);
	}

	VertexArray::~VertexArray()
	{
		AR_PROFILE_FUNCTION();

		glDeleteVertexArrays(1, &m_ArrayId);
	}

	void VertexArray::bind() const
	{
		AR_PROFILE_FUNCTION();

		glBindVertexArray(m_ArrayId);
	}

	void VertexArray::unBind() const
	{
		glBindVertexArray(0);
	}

	void VertexArray::addVertexBuffer(const Ref<VertexBuffer>& vertexBuffer)
	{
		AR_PROFILE_FUNCTION();

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
		AR_PROFILE_FUNCTION();

		glBindVertexArray(m_ArrayId);
		indexBuffer->bind();

		m_IndexBuffer = indexBuffer;
	}

}