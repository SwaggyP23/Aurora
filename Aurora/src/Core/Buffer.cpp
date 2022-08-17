#include "Aurorapch.h"
#include "Buffer.h"

namespace Aurora {

	Buffer::Buffer(size_t sizeInBytes)
	{
	}

	Buffer::Buffer(const Buffer& other)
	{
	}

	Buffer::Buffer(Buffer&& other) noexcept
	{
	}

	Buffer::~Buffer()
	{

	}

	Buffer& Buffer::operator=(const Buffer& other)
	{
		return Buffer{};
		// TODO: insert return statement here
	}

	Buffer& Buffer::operator=(Buffer&& other) noexcept
	{
		return Buffer{};
		// TODO: insert return statement here
	}

	Byte* Buffer::Allocate(size_t sizeInBytes)
	{
		return nullptr;
	}

	void Buffer::Deallocate()
	{
	}

	void Buffer::Reset()
	{
	}

}