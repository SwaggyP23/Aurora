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
		IndexBuffer(uint32_t* indices, uint32_t count);
		~IndexBuffer();

		static Ref<IndexBuffer> Create();
		static Ref<IndexBuffer> Create(uint32_t* indices, uint32_t count);

		void Bind() const;
		void UnBind() const;

		inline uint32_t GetCount() const { return m_Count; }

	private:
		uint32_t m_BufferId = 0;
		uint32_t m_Count = 0;

	};

}