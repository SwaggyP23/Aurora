#pragma once

#include "Core/Base.h"

namespace Aurora {

	class UniformBuffer : public RefCountedObject
	{
	public:
		UniformBuffer(uint32_t size, uint32_t binding);
		UniformBuffer(uint32_t size, uint32_t binding, const void* data);
		virtual ~UniformBuffer();

		[[nodiscard]] static Ref<UniformBuffer> Create(uint32_t size, uint32_t binding);
		[[nodiscard]] static Ref<UniformBuffer> Create(uint32_t size, uint32_t binding, const void* data);

		void SetData(const void* data, uint32_t size, uint32_t offset = 0);

		[[nodiscard]] uint32_t GetSize() const { return m_Size; }
		[[nodiscard]] uint32_t GetBinding() const { return m_BindingPoint; }

	private:
		uint32_t m_BufferID = 0;
		uint32_t m_Size = 0;
		uint32_t m_BindingPoint = 0;

	};

}