#include "Aurorapch.h"
#include "UniformBuffer.h"

#include <glad/glad.h>

namespace Aurora {

	Ref<UniformBuffer> UniformBuffer::Create(uint32_t size, uint32_t binding)
	{
		return CreateRef<UniformBuffer>(size, binding);
	}

	Ref<UniformBuffer> UniformBuffer::Create(uint32_t size, uint32_t binding, const void* data)
	{
		return CreateRef<UniformBuffer>(size, binding, data);
	}

	UniformBuffer::UniformBuffer(uint32_t size, uint32_t binding)
		: m_Size(size), m_BindingPoint(binding)
	{
		glCreateBuffers(1, &m_BufferID);
		glNamedBufferData(m_BufferID, size, nullptr, GL_DYNAMIC_DRAW);
		glBindBufferBase(GL_UNIFORM_BUFFER, binding, m_BufferID);
	}

	UniformBuffer::UniformBuffer(uint32_t size, uint32_t binding, const void* data)
		: m_Size(size), m_BindingPoint(binding)
	{
		glCreateBuffers(1, &m_BufferID);
		glNamedBufferData(m_BufferID, size, data, GL_DYNAMIC_DRAW);
		glBindBufferBase(GL_UNIFORM_BUFFER, binding, m_BufferID);
	}

	UniformBuffer::~UniformBuffer()
	{
		glDeleteBuffers(1, &m_BufferID);
	}

	void UniformBuffer::SetData(const void* data, uint32_t size, uint32_t offset)
	{
		glNamedBufferSubData(m_BufferID, offset, size, data);
	}

}