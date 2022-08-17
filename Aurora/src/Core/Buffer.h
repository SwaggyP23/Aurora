#pragma once

#include "Base.h"

/*
 * Just a buffer of plain bytes, could hold anything 
 */

namespace Aurora {

	class Buffer
	{
	public:
		constexpr Buffer() = default;
		Buffer(size_t sizeInBytes);
		Buffer(const Buffer& other);
		Buffer(Buffer&& other) noexcept;
		~Buffer();

		Byte* Allocate(size_t sizeInBytes);
		void Deallocate();

		void Reset();

		Buffer& operator=(const Buffer& other);
		Buffer& operator=(Buffer&& other) noexcept;

	private:
		Byte* m_Data = nullptr;
		//uint32_t m_Size; ??

	};

}