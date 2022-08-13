#include "Aurorapch.h"
#include "UniformBuffer.h"

#include <glad/glad.h>

namespace Aurora {

	UnifromBuffer::UnifromBuffer(uint32_t size, uint32_t binding)
	{
		m_LocalStorage = new Byte[size];

		glCreateBuffers(1, &m_BufferID);
		glNamedBufferData(m_BufferID, size, nullptr, GL_DYNAMIC_DRAW);
		glBindBufferBase(GL_UNIFORM_BUFFER, binding, m_BufferID);
	}

	UnifromBuffer::~UnifromBuffer()
	{
		delete[] m_LocalStorage;

		glDeleteBuffers(1, &m_BufferID);
	}

	void UnifromBuffer::SetData(const void* data, uint32_t size, uint32_t offset)
	{
		glNamedBufferSubData(m_BufferID, offset, size, data);
	}

}