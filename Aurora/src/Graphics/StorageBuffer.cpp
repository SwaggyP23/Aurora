#include "Aurorapch.h"
#include "StorageBuffer.h"

#include <glad/glad.h>

namespace Aurora {

	Ref<StorageBuffer> StorageBuffer::Create(uint32_t size, uint32_t binding)
	{
		return CreateRef<StorageBuffer>(size, binding);
	}

	StorageBuffer::StorageBuffer(uint32_t size, uint32_t binding)
		: m_Size(size), m_BindingPoint(binding)
	{
		glCreateBuffers(1, &m_BufferID);
		glNamedBufferData(m_BufferID, size, nullptr, GL_DYNAMIC_DRAW);
		glBindBufferBase(GL_UNIFORM_BUFFER, binding, m_BufferID);
	}

	StorageBuffer::~StorageBuffer()
	{
		glDeleteBuffers(1, &m_BufferID);
	}

	void StorageBuffer::SetData(const void* data, uint32_t size, uint32_t offset)
	{
		glNamedBufferSubData(m_BufferID, offset, size, data);
	}

}