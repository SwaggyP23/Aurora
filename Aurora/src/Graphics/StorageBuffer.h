#pragma once

#include "Core/Base.h"

namespace Aurora {

	class StorageBuffer : public RefCountedObject
	{
	public:
		StorageBuffer(uint32_t size, uint32_t binding);
		~StorageBuffer();

		static Ref<StorageBuffer> Create(uint32_t size, uint32_t binding);

		void SetData(const void* data, uint32_t size, uint32_t offset = 0);

		uint32_t GetSize() const { return m_Size; }
		uint32_t GetBinding() const { return m_BindingPoint; }

	private:
		uint32_t m_BufferID = 0;
		uint32_t m_Size = 0;
		uint32_t m_BindingPoint = 0;

	};

}