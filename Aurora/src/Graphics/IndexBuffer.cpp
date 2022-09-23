#include "Aurorapch.h"
#include "IndexBuffer.h"

#include <glad/glad.h>

namespace Aurora {

	//////////////////////////
    // INDEX BUFFER!!
    //////////////////////////

	Ref<IndexBuffer> IndexBuffer::Create()
	{
		return CreateRef<IndexBuffer>();
	}

	Ref<IndexBuffer> IndexBuffer::Create(void* indices, uint32_t size)
	{
		return CreateRef<IndexBuffer>(indices, size);
	}

	IndexBuffer::IndexBuffer(void* indices, uint32_t size)
		: m_Size(size)
	{
		glCreateBuffers(1, &m_BufferId);
		// Since we specified here that it is a GL_ELEMENT_ARRAY_BUFFER, a VAO must be bound when this is created 
		// otherwise it will not work
		glNamedBufferData(m_BufferId, size, indices, GL_STATIC_DRAW);
	}

	IndexBuffer::~IndexBuffer()
	{
		glDeleteBuffers(1, &m_BufferId);
	}

	void IndexBuffer::Bind() const
	{
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_BufferId);
	}

	void IndexBuffer::UnBind() const
	{
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}

}