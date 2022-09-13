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
		Buffer(Byte* data, size_t sizeInBytes);
		Buffer(const Buffer& other);
		Buffer(Buffer&& other) noexcept;
		~Buffer();

		Buffer& operator=(const Buffer& other);
		Buffer& operator=(Buffer&& other) noexcept;

		void Allocate(size_t sizeInBytes);
		void Release();

		void Write(const void* data, size_t size, uint32_t offset = 0);
		void Reverse();

		template<typename T>
		T& Read(uint32_t offset = 0)
		{
			return *(T*)(Data + offset);
		}

		operator bool() const { return Data; }

		Byte& operator[](int index) { return ((Byte*)Data)[index]; }
		const Byte& operator[](int index) const { return ((Byte*)Data)[index]; }

	public:
		Byte* Data = nullptr;
		size_t Size = 0;
	};

}