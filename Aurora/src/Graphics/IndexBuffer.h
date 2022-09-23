#pragma once

#include "Core/Base.h"

namespace Aurora {

	//////////////////////////
    // INDEX BUFFER!!
    //////////////////////////

    // TODO: Only supports 32-bit index buffers, should add support for 16-bit index buffers
	class IndexBuffer : public RefCountedObject
	{
	public:
		IndexBuffer() = default;
		IndexBuffer(void* indices, uint32_t size);
		~IndexBuffer();

		[[nodiscard]] static Ref<IndexBuffer> Create();
		[[nodiscard]] static Ref<IndexBuffer> Create(void* indices, uint32_t size);

		void Bind() const;
		void UnBind() const;

		[[nodiscard]] inline uint32_t GetSize() const { return m_Size; }
		[[nodiscard]] inline uint32_t GetBufferID() const { return m_BufferId; }

	private:
		uint32_t m_BufferId = 0;
		uint32_t m_Size = 0;

	};

}