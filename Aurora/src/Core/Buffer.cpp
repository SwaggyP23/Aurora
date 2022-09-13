#include "Aurorapch.h"
#include "Buffer.h"

namespace Aurora {

	Buffer::Buffer(size_t sizeInBytes)
		: Data(nullptr), Size(sizeInBytes)
	{
		Allocate(sizeInBytes);
	}

	Buffer::Buffer(Byte* data, size_t sizeInBytes)
		: Data(data), Size(sizeInBytes)
	{
		AR_CORE_CHECK(sizeInBytes > 0);
	}

	Buffer::Buffer(const Buffer& other)
		: Data(nullptr), Size(other.Size)
	{
		AR_CORE_CHECK(other.Size > 0);

		Data = new Byte[other.Size];
		memcpy(Data, other.Data, other.Size);
	}

	Buffer::Buffer(Buffer&& other) noexcept
		: Data(other.Data), Size(other.Size)
	{
		AR_CORE_CHECK(other.Size > 0);

		other.Data = nullptr;
		other.Size = 0;
	}

	Buffer::~Buffer()
	{
		Release();
	}

	Buffer& Buffer::operator=(const Buffer& other)
	{
		// Have to release incase the buffer already was owning some memory
		Release();

		Data = new Byte[other.Size];
		Size = other.Size;
		
		return *this;
	}

	Buffer& Buffer::operator=(Buffer&& other) noexcept
	{
		Data = other.Data;
		Size = other.Size;

		other.Data = nullptr;
		other.Size = 0;

		return *this;
	}

	void Buffer::Allocate(size_t sizeInBytes)
	{
		AR_CORE_CHECK(sizeInBytes > 0);

		if (Data)
			Release();

		Data = new Byte[sizeInBytes];
		memset(Data, 0, sizeInBytes);

		Size = sizeInBytes;
	}

	void Buffer::Release()
	{
		if (Data)
		{
			delete[] Data;
			Data = nullptr;
		}

		Size = 0;
	}

	void Buffer::Write(const void* data, size_t size, uint32_t offset)
	{
		AR_CORE_ASSERT(offset + size <= Size, "Buffer Overflow!");

		memcpy(Data + offset, data, size);
	}

	void Buffer::Reverse()
	{
		uint8_t* newData = new uint8_t[Size];
		memset(newData, 0, Size);

		for (size_t i = 0; i < Size; i++)
		{
			newData[i] = Data[Size - 1 - i];
		}

		memcpy(Data, newData, Size);
	}

}