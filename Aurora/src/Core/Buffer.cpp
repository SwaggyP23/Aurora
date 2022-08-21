#include "Aurorapch.h"
#include "Buffer.h"

namespace Aurora {

	Buffer::Buffer(void* data, uint32_t size)
		: m_Data(data), m_Size(size)
	{
	}

	Buffer::Buffer(size_t size)
		: m_Size((uint32_t)size)
	{
		ZeroInit();
	}

	Buffer::Buffer(const Buffer& other)
	{
		Allocate(other.m_Size);
		memcpy(m_Data, other.m_Data, other.m_Size);
	}

	Buffer::Buffer(Buffer&& other) noexcept
	{
		m_Data = other.m_Data;
		m_Size = other.m_Size;

		other.m_Data = nullptr;
		other.m_Size = 0;
	}

	Buffer::~Buffer()
	{
		Release();
	}

	Buffer& Buffer::operator=(const Buffer& other)
	{
		Allocate(other.m_Size);
		memcpy(m_Data, other.m_Data, other.m_Size);

		return *this;
	}

	Buffer& Buffer::operator=(Buffer&& other) noexcept
	{
		m_Data = other.m_Data;
		m_Size = other.m_Size;

		other.m_Data = nullptr;
		other.m_Size = 0;

		return *this;
	}

	void Buffer::Allocate(size_t size)
	{
		if (m_Data)
		{
			delete[] m_Data;
			m_Data = nullptr;
			m_Size = 0;
		}

		if (size == 0)
			return;

		m_Data = new Byte[size];
		m_Size = (uint32_t)size;
	}

	void Buffer::Release()
	{
		delete[] m_Data;
		m_Data = nullptr;

		m_Size = 0;
	}

	void Buffer::ZeroInit()
	{
		if (m_Data)
			memset(m_Data, 0, m_Size);
	}

	void Buffer::Write(void* data, uint32_t size, uint32_t offset)
	{
		AR_CORE_ASSERT(offset + size <= m_Size, "Buffer Overflow!");

		memcpy((Byte*)m_Data + offset, data, size);
	}

	Byte* Buffer::ReadBytes(uint32_t size, uint32_t offset)
	{
		AR_CORE_ASSERT(offset + size <= m_Size, "Buffer Overflow!");

		Byte* buffer = new Byte[size];
		memcpy(buffer, (Byte*)m_Data + offset, size);

		return buffer;
	}

}