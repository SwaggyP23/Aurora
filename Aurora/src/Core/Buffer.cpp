#include "Aurorapch.h"
#include "Buffer.h"

namespace Aurora {

	Buffer::Buffer(void* data, uint32_t size)
		: m_Data(data), m_Size(size)
	{
	}

	Buffer Buffer::Copy(const void* data, uint32_t size)
	{
		Buffer newBuffer;
		newBuffer.Allocate(size);
		memcpy(newBuffer.GetData(), data, size);

		return newBuffer;
	}

	void Buffer::Allocate(size_t size)
	{
		delete[] m_Data;
		m_Data = nullptr;

		if (size == 0)
			return;

		m_Data = new Byte[size];
		m_Size = (uint32_t)size;
	}

	void Buffer::Release()
	{
		if (m_Data)
		{
			delete[] m_Data;
			m_Data = nullptr;
		}

		m_Size = 0;
	}

	void Buffer::ZeroInit()
	{
		if (m_Data)
			memset(m_Data, 0, m_Size);
	}

	Byte* Buffer::ReadBytes(uint32_t size, uint32_t offset)
	{
		AR_CORE_ASSERT(offset + size <= m_Size, "Buffer Overflow!");

		Byte* buffer = new Byte[size];
		memcpy(buffer, (Byte*)m_Data + offset, size);

		return buffer;
	}

	void Buffer::Write(void* data, uint32_t size, uint32_t offset)
	{
		AR_CORE_ASSERT(offset + size <= m_Size, "Buffer Overflow!");

		memcpy((Byte*)m_Data + offset, data, size);
	}

}