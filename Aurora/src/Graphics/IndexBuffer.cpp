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

	Ref<IndexBuffer> IndexBuffer::Create(uint32_t* indices, uint32_t count)
	{
		return CreateRef<IndexBuffer>(indices, count);
	}

	IndexBuffer::IndexBuffer(uint32_t* indices, uint32_t count)
		: m_Count(count)
	{
		AR_PROFILE_FUNCTION();

		glCreateBuffers(1, &m_BufferId);
		// Since we specified here that it is a GL_ELEMENT_ARRAY_BUFFER, a VAO must be bound when this is created 
		// otherwise it will not work
		glNamedBufferData(m_BufferId, count * sizeof(uint32_t), indices, GL_STATIC_DRAW);
	}

	IndexBuffer::~IndexBuffer()
	{
		AR_PROFILE_FUNCTION();

		glDeleteBuffers(1, &m_BufferId);
	}

	void IndexBuffer::Bind() const
	{
		AR_PROFILE_FUNCTION();

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_BufferId);
	}

	void IndexBuffer::UnBind() const
	{
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}

}