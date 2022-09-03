#pragma once

#include "Base.h"

/*
 * Just a buffer of plain bytes, could hold anything 
 */

namespace Aurora {

	class Buffer
	{
	public:
		Buffer() = default;
		Buffer(void* data, uint32_t size);

		static Buffer Copy(const void* data, uint32_t size);

		void Allocate(size_t sizeInBytes);
		void Release();

		void ZeroInit();
		void Write(void* data, uint32_t size, uint32_t offset = 0);
		Byte* ReadBytes(uint32_t size, uint32_t offset);

		template<typename T>
		T& Read(uint32_t offset = 0)
		{
			return *(T*)((Byte*)m_Data + offset);
		}

		inline void* GetData() { return m_Data; }
		inline uint32_t GetSize() const { return m_Size; }

		operator bool() const { return m_Data; }

		Byte& operator[](int index) { return ((Byte*)m_Data)[index]; }
		const Byte& operator[](int index) const { return ((Byte*)m_Data)[index]; }

	private:
		void* m_Data = nullptr;
		uint32_t m_Size = 0;

	};

}